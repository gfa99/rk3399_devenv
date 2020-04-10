/*
 * Copyright (C) 2019 Hertz Wang 1989wanghang@163.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see http://www.gnu.org/licenses
 *
 * Any non-GPL usage of this software or parts of this software is strictly
 * forbidden.
 *
 */

// this file run on 1808 with npu
// data path: camera (-> mpp) -> rga -> npu ->uvc

#ifndef DEBUG
#define DEBUG
#endif

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <easymedia/buffer.h>
#include <easymedia/key_string.h>
#include <easymedia/media_config.h>
#include <easymedia/utils.h>

#include <easymedia/flow.h>

static std::shared_ptr<easymedia::Flow>
create_flow(const std::string &flow_name, const std::string &flow_param,
            const std::string &elem_param);

extern "C" {
#include <uvc/mpi_enc.h>
#include <uvc/uvc_control.h>
#include <uvc/uvc_video.h>
}

#include "../npu_pp_output.h"
#include <SDL2/SDL.h>

#include <camera_engine_rkisp/interface/mediactl.h>
#include <camera_engine_rkisp/interface/rkisp_control_loop.h>

#include "npu_uvc_shared.h"
#include <rknn_runtime.h>
static bool do_uvc(easymedia::Flow *f,
                   easymedia::MediaBufferVector &input_vector);
class UVCJoinFlow : public easymedia::Flow {
public:
  UVCJoinFlow(uint32_t npu_output_type, const std::string &model,
              uint32_t npu_w, uint32_t npu_h, bool render);
  virtual ~UVCJoinFlow() {
    StopAllThread();
    uvc_control_join(UVC_CONTROL_LOOP_ONCE);
  }
  bool Init();

private:
  uint32_t npu_output_type;
  std::string model_identifier;
  uint32_t npu_width;
  uint32_t npu_height;
  bool render_npu_result;
  friend bool do_uvc(easymedia::Flow *f,
                     easymedia::MediaBufferVector &input_vector);
};

UVCJoinFlow::UVCJoinFlow(uint32_t type, const std::string &model,
                         uint32_t npu_w, uint32_t npu_h, bool render)
    : npu_output_type(type), model_identifier(model), npu_width(npu_w),
      npu_height(npu_h), render_npu_result(render) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(2);
  sm.fetch_block.push_back(true);
  if (!render) {
    sm.input_slots.push_back(1);
    sm.input_maxcachenum.push_back(1);
    sm.fetch_block.push_back(false);
  }
  sm.process = do_uvc;
  if (!InstallSlotMap(sm, "uvc_extract", -1)) {
    fprintf(stderr, "Fail to InstallSlotMap, %s\n", "uvc_join");
    SetError(-EINVAL);
    return;
  }
}

bool UVCJoinFlow::Init() {
  uvc_control_run(UVC_CONTROL_LOOP_ONCE);
  return true;
}

static MppFrameFormat ConvertToMppPixFmt(const PixelFormat &fmt) {
  static_assert(PIX_FMT_YUV420P == 0, "The index should greater than 0\n");
  static MppFrameFormat mpp_fmts[PIX_FMT_NB] = {
      [PIX_FMT_YUV420P] = MPP_FMT_YUV420P,
      [PIX_FMT_NV12] = MPP_FMT_YUV420SP,
      [PIX_FMT_NV21] = MPP_FMT_YUV420SP_VU,
      [PIX_FMT_YUV422P] = MPP_FMT_YUV422P,
      [PIX_FMT_NV16] = MPP_FMT_YUV422SP,
      [PIX_FMT_NV61] = MPP_FMT_YUV422SP_VU,
      [PIX_FMT_YUYV422] = MPP_FMT_YUV422_YUYV,
      [PIX_FMT_UYVY422] = MPP_FMT_YUV422_UYVY,
      [PIX_FMT_RGB332] = (MppFrameFormat)-1,
      [PIX_FMT_RGB565] = MPP_FMT_RGB565,
      [PIX_FMT_BGR565] = MPP_FMT_BGR565,
      [PIX_FMT_RGB888] = MPP_FMT_RGB888,
      [PIX_FMT_BGR888] = MPP_FMT_BGR888,
      [PIX_FMT_ARGB8888] = MPP_FMT_ARGB8888,
      [PIX_FMT_ABGR8888] = MPP_FMT_ABGR8888};
  if (fmt >= 0 && fmt < PIX_FMT_NB)
    return mpp_fmts[fmt];
  return (MppFrameFormat)-1;
}

static struct extra_jpeg_data *
serialize(struct extra_jpeg_data &ejd,
          std::shared_ptr<easymedia::MediaBuffer> &npu_output_buf,
          size_t &size) {
  struct extra_jpeg_data *new_ejd = nullptr;
  switch (ejd.npu_output_type) {
  case TYPE_RK_NPU_OUTPUT: {
    size_t num = npu_output_buf->GetValidSize();
    ejd.npu_outputs_num = num;
    rknn_output *outputs = (rknn_output *)npu_output_buf->GetPtr();
    ejd.npu_outputs_timestamp = npu_output_buf->GetUSTimeStamp();
    ejd.npu_output_size = num * sizeof(struct aligned_npu_output);
    for (size_t i = 0; i < num; i++)
      ejd.npu_output_size += outputs[i].size;
    size = sizeof(ejd) + ejd.npu_output_size;
    new_ejd = (struct extra_jpeg_data *)malloc(size);
    if (!new_ejd)
      return nullptr;
    *new_ejd = ejd;
    uint32_t pos = 0;
    for (size_t i = 0; i < num; i++) {
      struct aligned_npu_output *an =
          (struct aligned_npu_output *)(new_ejd->outputs + pos);
      an->want_float = outputs[i].want_float;
      an->is_prealloc = outputs[i].is_prealloc;
      an->index = outputs[i].index;
      an->size = outputs[i].size;
      memcpy(an->buf, outputs[i].buf, outputs[i].size);
      pos += (sizeof(*an) + outputs[i].size);
    }
  } break;
  case TYPE_RK_ROCKX_OUTPUT: {
    size_t num = npu_output_buf->GetValidSize();
    ejd.npu_outputs_num = num;
    // rknn_output *outputs = (rknn_output *)npu_output_buf->GetPtr();
    ejd.npu_outputs_timestamp = npu_output_buf->GetUSTimeStamp();
    ejd.npu_output_size = npu_output_buf->GetSize();
    size = sizeof(ejd) + ejd.npu_output_size;
    new_ejd = (struct extra_jpeg_data *)malloc(size);
    if (!new_ejd)
      return nullptr;
    *new_ejd = ejd;
    memcpy(new_ejd->outputs, npu_output_buf->GetPtr(), ejd.npu_output_size);
  } break;
  default:
    fprintf(stderr, "unimplemented rk nn output type: %d\n",
            ejd.npu_output_type);
  }
  return new_ejd;
}

bool do_uvc(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector) {
  UVCJoinFlow *flow = (UVCJoinFlow *)f;
  auto img_buf = input_vector[0];
  if (!img_buf || img_buf->GetType() != Type::Image)
    return false;
  std::shared_ptr<easymedia::MediaBuffer> npu_output_buf;
  if (flow->render_npu_result) {
    auto vec = img_buf->GetRelatedSPtrs();
    if (vec.size() > 0)
      npu_output_buf = std::static_pointer_cast<easymedia::MediaBuffer>(vec[0]);
  } else {
    npu_output_buf = input_vector[1];
  }
  auto img = std::static_pointer_cast<easymedia::ImageBuffer>(img_buf);
  MppFrameFormat ifmt = ConvertToMppPixFmt(img->GetPixelFormat());
  assert(ifmt == MPP_FMT_YUV420SP);
  // fprintf(stderr, "ifmt: %d,size: %d\n", ifmt, (int)img_buf->GetValidSize());
  if (ifmt < 0)
    return false;
  mpi_enc_set_format(ifmt);
  struct extra_jpeg_data ejd;
  ejd.picture_timestamp = img_buf->GetUSTimeStamp();
  ejd.npu_output_type = flow->npu_output_type;
  snprintf((char *)ejd.model_identifier, sizeof(ejd.model_identifier),
           flow->model_identifier.c_str());
  assert(sizeof(ejd) >= 4); // fake ffe2, set it must more or equal 4
  if (!npu_output_buf) {
    ejd.npu_outputs_timestamp = 0;
    ejd.npu_output_size = 0;
    ejd.npu_outputs_num = 0;
    uvc_read_camera_buffer(img_buf->GetPtr(), img_buf->GetFD(), img_buf->GetValidSize(),
			   &ejd, sizeof(ejd));
  } else {
    size_t size = 0;
    ejd.npuwh.width = flow->npu_width;
    ejd.npuwh.height = flow->npu_height;
    struct extra_jpeg_data *new_ejd = serialize(ejd, npu_output_buf, size);
    if (new_ejd) {
      // fprintf(stderr, "set extra uvc data: %p, size: %d\n", new_ejd,
      // (int)size);
      uvc_read_camera_buffer(img_buf->GetPtr(), img_buf->GetFD(), img_buf->GetValidSize(),
                             new_ejd, size);
      free(new_ejd);
    } else {
      return false;
    }
  }
  return true;
}

#if HAVE_ROCKX
#include <rockx.h>
static bool do_rockx(easymedia::Flow *f,
                     easymedia::MediaBufferVector &input_vector);
class RockxFlow : public easymedia::Flow {
public:
  RockxFlow(const std::string &model, bool hand_over_big_pic);
  virtual ~RockxFlow() {
    StopAllThread();
    for (auto handle : rockx_handles)
      rockx_destroy(handle);
  }

private:
  std::string model_identifier;
  std::vector<rockx_handle_t> rockx_handles;
  std::shared_ptr<easymedia::MediaBuffer> tmp_img;
  bool hand_over_big_pic;
  friend bool do_rockx(easymedia::Flow *f,
                       easymedia::MediaBufferVector &input_vector);
};

RockxFlow::RockxFlow(const std::string &model_str, bool hand_over_pic)
    : model_identifier(model_str), hand_over_big_pic(hand_over_pic) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(2);
  sm.fetch_block.push_back(true);
  sm.output_slots.push_back(0);
  if (hand_over_big_pic) {
    sm.hold_input.push_back(easymedia::HoldInputMode::INHERIT_FORM_INPUT);
  }
  sm.process = do_rockx;
  if (!InstallSlotMap(sm, "rockx", -1)) {
    fprintf(stderr, "Fail to InstallSlotMap, %s\n", "rockx");
    SetError(-EINVAL);
    return;
  }
  std::vector<rockx_module_t> models;
  void *config = nullptr;
  size_t config_size = 0;
  if (model_str == "rockx_face_gender_age") {
    models.push_back(ROCKX_MODULE_FACE_DETECTION);
    models.push_back(ROCKX_MODULE_FACE_LANDMARK_5);
    models.push_back(ROCKX_MODULE_FACE_ANALYZE);
  } else if (model_str == "rockx_face_detect") {
    models.push_back(ROCKX_MODULE_FACE_DETECTION);
  } else {
    assert(0);
  }
  for (size_t i = 0; i < models.size(); i++) {
    rockx_handle_t npu_handle = nullptr;
    rockx_module_t &model = models[i];
    rockx_ret_t ret = rockx_create(&npu_handle, model, config, config_size);
    if (ret != ROCKX_RET_SUCCESS) {
      fprintf(stderr, "init rockx module %d error=%d\n", model, ret);
      SetError(-EINVAL);
      return;
    }
    rockx_handles.push_back(npu_handle);
  }
}

bool do_rockx(easymedia::Flow *f, easymedia::MediaBufferVector &input_vector) {
  assert(sizeof(float) == 4);
  RockxFlow *flow = (RockxFlow *)f;
  auto input =
      std::static_pointer_cast<easymedia::ImageBuffer>(input_vector[0]);
  if (!input)
    return false;
  rockx_image_t input_image;
  input_image.width = input->GetWidth();
  input_image.height = input->GetHeight();
  input_image.data = (uint8_t *)input->GetPtr();
  input_image.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
  auto &name = flow->model_identifier;
  auto &handles = flow->rockx_handles;
  if (name == "rockx_face_gender_age") {
    auto &tmp_img = flow->tmp_img;
    if (!tmp_img) {
      tmp_img = easymedia::MediaBuffer::Alloc(112 * 112 * 3 * 4);
      if (!tmp_img) {
        fprintf(stderr, "no memory\n");
        return false;
      }
    }
    rockx_handle_t &face_det_handle = handles[0];
    rockx_handle_t &face_5landmarks_handle = handles[1];
    rockx_handle_t &face_attribute_handle = handles[2];
    rockx_object_array_t face_array;
    memset(&face_array, 0, sizeof(rockx_object_array_t));
    rockx_ret_t ret =
        rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
      fprintf(stderr, "rockx_face_detect error %d\n", ret);
      return false;
    }
    if (face_array.count <= 0)
      return false;
    rockx_image_t out_img;
    out_img.width = 112;
    out_img.height = 112;
    out_img.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
    out_img.data = (uint8_t *)tmp_img->GetPtr();
    size_t ret_buf_size =
        face_array.count * sizeof(struct aligned_rockx_face_gender_age);
    rockx_face_attribute_t gender_age;
    memset(&gender_age, 0, sizeof(gender_age));
    auto ret_buf = easymedia::MediaBuffer::Alloc(ret_buf_size);
    if (!ret_buf)
      return false;
    auto face_attribute_array =
        (struct aligned_rockx_face_gender_age *)ret_buf->GetPtr();
    memset(face_attribute_array, 0, ret_buf_size);
    size_t count = 0;
    auto big = std::static_pointer_cast<easymedia::ImageBuffer>(
        input->GetRelatedSPtrs()[0]);
    assert(big);
    rockx_image_t big_img;
    big_img.width = big->GetWidth();
    big_img.height = big->GetHeight();
    big_img.pixel_format = ROCKX_PIXEL_FORMAT_RGB888;
    big_img.data = (uint8_t *)big->GetPtr();
    assert(big_img.data);
    for (int i = 0; i < face_array.count; i++) {
      if (face_array.object[i].score < 0.85)
        continue; // save cpu
      auto array = &face_attribute_array[count];
      array->left = face_array.object[i].box.left;
      array->top = face_array.object[i].box.top;
      array->right = face_array.object[i].box.right;
      array->bottom = face_array.object[i].box.bottom;
      fprintf(stderr, "[%d]: %d,%d - %d,%d\n", i, array->left, array->top,
              array->right, array->bottom);
      rockx_rect_t crop_rect;
      crop_rect.left = array->left * big_img.width / input_image.width;
      crop_rect.top = array->top * big_img.height / input_image.height;
      crop_rect.right = array->right * big_img.width / input_image.width;
      crop_rect.bottom = array->bottom * big_img.height / input_image.height;
      ret = rockx_face_align(face_5landmarks_handle, &big_img, &crop_rect,
                             nullptr, &out_img);
      if (ret != ROCKX_RET_SUCCESS) {
        fprintf(stderr, "rockx_face_align error %d\n", ret);
        continue;
      }
      ret = rockx_face_attribute(face_attribute_handle, &out_img, &gender_age);
      if (ret != ROCKX_RET_SUCCESS) {
        fprintf(stderr, "rockx_face_attribute error %d\n", ret);
        continue;
      }
      if (false) {
        rockx_image_write("/data/big.jpg", &big_img);
        rockx_image_write("/data/small.jpg", &input_image);
        rockx_image_write("/data/aligned_face.jpg", &out_img);
      }
      memcpy(array->score, &face_array.object[i].score, 4);
      array->gender = gender_age.gender;
      array->age = gender_age.age;
      count++;
    }
    if (count == 0)
      return false;
    ret_buf->SetSize(count * sizeof(struct aligned_rockx_face_gender_age));
    ret_buf->SetValidSize(count);
    return flow->SetOutput(ret_buf, 0);
  } else if (name == "rockx_face_detect") {
    rockx_handle_t &face_det_handle = handles[0];
    rockx_object_array_t face_array;
    memset(&face_array, 0, sizeof(rockx_object_array_t));
    rockx_ret_t ret =
        rockx_face_detect(face_det_handle, &input_image, &face_array, nullptr);
    if (ret != ROCKX_RET_SUCCESS) {
      fprintf(stderr, "rockx_face_detect error %d\n", ret);
      return false;
    }
    if (face_array.count <= 0)
      return false;
    size_t ret_buf_size =
        face_array.count * sizeof(struct aligned_rockx_face_rect);
    auto ret_buf = easymedia::MediaBuffer::Alloc(ret_buf_size);
    if (!ret_buf)
      return false;
    auto face_rects = (struct aligned_rockx_face_rect *)ret_buf->GetPtr();
    memset(face_rects, 0, ret_buf_size);
    for (int i = 0; i < face_array.count; i++) {
      auto array = &face_rects[i];
      array->left = face_array.object[i].box.left;
      array->top = face_array.object[i].box.top;
      array->right = face_array.object[i].box.right;
      array->bottom = face_array.object[i].box.bottom;
      memcpy(array->score, &face_array.object[i].score, 4);
    }
    ret_buf->SetSize(ret_buf_size);
    ret_buf->SetValidSize(face_array.count);
    return flow->SetOutput(ret_buf, 0);
  } else {
    assert(0);
  }
  return true;
}

#endif // #if HAVE_ROCKX

static bool do_sdl_draw(easymedia::Flow *f,
                        easymedia::MediaBufferVector &input_vector);
class SDLDrawFlow : public easymedia::Flow {
public:
  SDLDrawFlow(uint32_t type, const std::string &model, uint32_t npu_w,
              uint32_t npu_h)
      : npu_output_type(type), model_identifier(model), npu_width(npu_w),
        npu_height(npu_h) {
    easymedia::SlotMap sm;
    sm.thread_model = easymedia::Model::ASYNCATOMIC;
    sm.mode_when_full = easymedia::InputMode::DROPFRONT;
    sm.input_slots.push_back(0);
    sm.input_maxcachenum.push_back(1);
    sm.output_slots.push_back(0);
    sm.process = do_sdl_draw;
    if (!InstallSlotMap(sm, "sdl_draw", -1)) {
      fprintf(stderr, "Fail to InstallSlotMap, %s\n", "sdl_draw");
      SetError(-EINVAL);
      return;
    }
  }
  virtual ~SDLDrawFlow() {
    StopAllThread();
    fprintf(stderr, "sdl draw flow quit\n");
  }

  uint32_t npu_output_type;
  std::string model_identifier;
  uint32_t npu_width;
  uint32_t npu_height;
  friend bool do_sdl_draw(easymedia::Flow *f,
                          easymedia::MediaBufferVector &input_vector);
};

bool do_sdl_draw(easymedia::Flow *f,
                 easymedia::MediaBufferVector &input_vector) {
  auto flow = static_cast<SDLDrawFlow *>(f);
  auto &npu_output_buf = input_vector[0];
  if (!npu_output_buf)
    return false;
  bool ret = false;
  auto img = std::static_pointer_cast<easymedia::ImageBuffer>(
      npu_output_buf->GetRelatedSPtrs()[0]);
  static Uint32 sdl_fmt = SDL_PIXELFORMAT_RGB24;
  SDL_Surface *surface = nullptr;
  SDL_Renderer *renderer = nullptr;
  static SDL_Rect dst_rect = {0, 0, img->GetWidth(), img->GetHeight()};
  struct extra_jpeg_data ejd;
  struct extra_jpeg_data *new_ejd = nullptr;
  size_t size = 0;
  if (!npu_output_buf->IsValid())
    goto out;
  ejd.picture_timestamp = img->GetUSTimeStamp();
  ejd.npu_output_type = flow->npu_output_type;
  snprintf((char *)ejd.model_identifier, sizeof(ejd.model_identifier),
           flow->model_identifier.c_str());
  ejd.npuwh.width = flow->npu_width;
  ejd.npuwh.height = flow->npu_height;
  new_ejd = serialize(ejd, npu_output_buf, size);
  if (!new_ejd)
    goto out;
  SDL_LogSetPriority(SDL_LOG_CATEGORY_VIDEO, SDL_LOG_PRIORITY_VERBOSE);
  SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_VERBOSE);
  surface = SDL_CreateRGBSurfaceWithFormatFrom(img->GetPtr(), img->GetWidth(),
                                               img->GetHeight(), 24,
                                               img->GetWidth() * 3, sdl_fmt);
  if (!surface) {
    fprintf(stderr, "SDL_CreateRGBSurfaceWithFormatFrom failed at line %d\n",
            __LINE__);
    goto err;
  }
  renderer = SDL_CreateSoftwareRenderer(surface);
  if (!renderer) {
    fprintf(stderr, "SDL_CreateSoftwareRenderer failed at line %d\n", __LINE__);
    goto err;
  }
  do {
    NPUPostProcessOutput npo(new_ejd);
    (npo.pp_func)(renderer, dst_rect, dst_rect, 0, &npo, img->GetPtr(),
                  sdl_fmt);
  } while (0);
  SDL_RenderPresent(renderer);

out:
  npu_output_buf->GetRelatedSPtrs().clear();
  img->SetRelatedSPtr(npu_output_buf, 0);
  ret = flow->SetOutput(img, 0);

err:
  if (new_ejd)
    free(new_ejd);
  if (renderer)
    SDL_DestroyRenderer(renderer);
  if (surface)
    SDL_FreeSurface(surface);
  return ret;
}

struct RKisp_media_ctl {
  /* media controller */
  struct media_device *controller;
  struct media_entity *isp_subdev;
  struct media_entity *isp_params_dev;
  struct media_entity *isp_stats_dev;
  struct media_entity *sensor_subdev;
};

// rkisp* implement from external/camera_engine_rkisp
#define MAX_MEDIA_INDEX 64
static struct media_device *__rkisp_get_media_dev_by_vnode(const char *vnode) {
  char sys_path[64];
  struct media_device *device = nullptr;
  uint32_t nents, j, i = 0;
  FILE *fp;

  while (i < MAX_MEDIA_INDEX) {
    snprintf(sys_path, 64, "/dev/media%d", i++);
    fp = fopen(sys_path, "r");
    if (!fp)
      continue;
    fclose(fp);

    device = media_device_new(sys_path);
    /* Enumerate entities, pads and links. */
    media_device_enumerate(device);
    nents = media_get_entities_count(device);
    for (j = 0; j < nents; ++j) {
      struct media_entity *entity = media_get_entity(device, j);
      const char *devname = media_entity_get_devname(entity);
      if (NULL != devname) {
        if (!strcmp(devname, vnode))
          goto out;
      }
    }
    media_device_unref(device);
    device = nullptr;
  }

out:
  return device;
}

static void camera_engine_rkisp_stop(void *rkisp_engine) {
  if (!rkisp_engine)
    return;
  rkisp_cl_stop(rkisp_engine);
  rkisp_cl_deinit(rkisp_engine);
}

static void *camera_engine_rkisp_start(const std::string &device_name) {
  /* start isp */
  static const char *iq_file = "/etc/cam_iq.xml";
  struct RKisp_media_ctl rkisp;
  memset(&rkisp, 0, sizeof(rkisp));
  void *rkisp_engine = nullptr;
  rkisp_cl_init(&rkisp_engine, iq_file, nullptr);

  if (!rkisp_engine) {
    fprintf(stderr, "rkisp_cl_init engine failed\n");
    return nullptr;
  }

  struct rkisp_cl_prepare_params_s params;
  memset(&params, 0, sizeof(params));
  int nents;

  rkisp.controller = __rkisp_get_media_dev_by_vnode(device_name.c_str());
  if (!rkisp.controller) {
    fprintf(stderr, "Can't find controller, maybe use a wrong video-node or "
                    "wrong permission to media node");
    return nullptr;
  }
  rkisp.isp_subdev = media_get_entity_by_name(
      rkisp.controller, "rkisp1-isp-subdev", strlen("rkisp1-isp-subdev"));
  rkisp.isp_params_dev = media_get_entity_by_name(
      rkisp.controller, "rkisp1-input-params", strlen("rkisp1-input-params"));
  rkisp.isp_stats_dev = media_get_entity_by_name(
      rkisp.controller, "rkisp1-statistics", strlen("rkisp1-statistics"));
  /* assume the last enity is sensor_subdev */
  nents = media_get_entities_count(rkisp.controller);
  rkisp.sensor_subdev = media_get_entity(rkisp.controller, nents - 1);

  params.isp_sd_node_path = media_entity_get_devname(rkisp.isp_subdev);
  params.isp_vd_params_path = media_entity_get_devname(rkisp.isp_params_dev);
  params.isp_vd_stats_path = media_entity_get_devname(rkisp.isp_stats_dev);
  params.sensor_sd_node_path = media_entity_get_devname(rkisp.sensor_subdev);
  rkisp_cl_prepare(rkisp_engine, &params);

  media_device_unref(rkisp.controller);

  auto ret = rkisp_cl_start(rkisp_engine);
  if (ret) {
    camera_engine_rkisp_stop(rkisp_engine);
    rkisp_engine = nullptr;
  }

  if (!rkisp_engine) {
    fprintf(stderr, "rkisp_init engine failed\n");
    return nullptr;
  }
  fprintf(stderr, "rkisp_init engine succeed\n");
  return rkisp_engine;
}

#include "term_help.h"

// i : input path of camera
// c : need 3a
// w : input width of camera
// h : input height of camera
// f : input format of camera
// m : npu model path, if normal rknn api, not rockx
// n : model_name with input w/h model needs
// r : render npu result on video picture
static char optstr[] = "?i:c:w:h:f:m:n:r:";

int main(int argc, char **argv) {
  int c;
  std::string v4l2_video_path;
  std::string format = IMAGE_NV12;
  int width = 1280, height = 720;
  int npu_width = 300, npu_height = 300;
  std::string model_path;
  std::string model_name;
  bool need_3a = false;
  bool render_result = false;

  opterr = 1;
  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
    case 'i':
      v4l2_video_path = optarg;
      printf("input path: %s\n", v4l2_video_path.c_str());
      break;
    case 'c':
      need_3a = !!atoi(optarg);
      break;
    case 'f':
      format = optarg;
      printf("input format: %s\n", format.c_str());
      break;
    case 'w':
      width = atoi(optarg);
      break;
    case 'h':
      height = atoi(optarg);
      break;
    case 'm':
      model_path = optarg;
      printf("model path: %s\n", model_path.c_str());
      break;
    case 'n': {
      char *s = strchr(optarg, ':');
      assert(s);
      *s = 0;
      model_name = optarg;
      printf("model name: %s\n", model_name.c_str());
      int ret = sscanf(s + 1, "%dx%d\n", &npu_width, &npu_height);
      assert(ret == 2);
    } break;
    case 'r':
      render_result = !!atoi(optarg);
      break;
    case '?':
    default:
      printf("help:\n\t-i: v4l2 capture device path\n");
      printf("\t-f: v4l2 capture format\n");
      printf("\t-w: v4l2 capture width\n");
      printf("\t-h: v4l2 capture height\n");
      printf("\t-m: model path\n");
      printf("\t-n: model name, model request width/height; such as "
             "ssd:300x300\n");
      printf("\nusage example: \n");
      printf("rk_npu_uvc_device -i /dev/video5 -f image:jpeg -w 1280 -h 720 -m "
             "/userdata/ssd_inception_v2.rknn -n ssd\n");
      exit(0);
    }
  }
  assert(!v4l2_video_path.empty());
  void *rkisp_engine = nullptr;
  if (need_3a) {
    rkisp_engine = camera_engine_rkisp_start(v4l2_video_path);
    if (!rkisp_engine)
      exit(EXIT_FAILURE);
  }
  // mpp
  std::shared_ptr<easymedia::Flow> decoder;
  // many usb camera decode out fmt do not match to rkmpp,
  // rga do convert
  std::shared_ptr<easymedia::Flow> rga0;
  if (format == IMAGE_JPEG || format == VIDEO_H264) {
    std::string flow_name("video_dec");
    std::string codec_name("rkmpp");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, codec_name);
    if (render_result)
      PARAM_STRING_APPEND_TO(flow_param, KEY_OUTPUT_HOLD_INPUT,
                             (int)easymedia::HoldInputMode::INHERIT_FORM_INPUT);
    std::string dec_param;
    PARAM_STRING_APPEND(dec_param, KEY_INPUTDATATYPE, format);
    // set output data type work only for jpeg, but except 1808
    // PARAM_STRING_APPEND(dec_param, KEY_OUTPUTDATATYPE, IMAGE_NV12);
    PARAM_STRING_APPEND_TO(dec_param, KEY_OUTPUT_TIMEOUT, 5000);
    decoder = create_flow(flow_name, flow_param, dec_param);
    if (!decoder) {
      assert(0);
      return -1;
    }
  }

  // uvc only support nv12
  if (decoder || format != IMAGE_NV12 || render_result) {
    std::string flow_name("filter");
    std::string filter_name("rkrga");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, filter_name);
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_ASYNCCOMMON);
    PixelFormat rga_out_pix_fmt = StringToPixFmt(IMAGE_NV12);
    ImageInfo out_img_info = {rga_out_pix_fmt, 0, 0, 0, 0};
    flow_param.append(easymedia::to_param_string(out_img_info, false));
    std::string rga_param;
    std::vector<ImageRect> v = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    PARAM_STRING_APPEND(rga_param, KEY_BUFFER_RECT,
                        easymedia::TwoImageRectToString(v));
    rga0 = create_flow(flow_name, flow_param, rga_param);
    if (!rga0) {
      assert(0);
      return -1;
    }
  }

  // rga
  bool rga_need_hold_input =
      (model_name == "rockx_face_gender_age") || render_result;
  std::shared_ptr<easymedia::Flow> rga1;
  if (rga_need_hold_input) {
    std::string flow_name("filter");
    std::string filter_name("rkrga");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, filter_name);
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_ASYNCCOMMON);
    PixelFormat rga_out_pix_fmt = StringToPixFmt(IMAGE_RGB888);
    ImageInfo out_img_info = {rga_out_pix_fmt, 0, 0, 0, 0};
    flow_param.append(easymedia::to_param_string(out_img_info, false));
    std::string rga_param;
    std::vector<ImageRect> v = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    PARAM_STRING_APPEND(rga_param, KEY_BUFFER_RECT,
                        easymedia::TwoImageRectToString(v));
    rga1 = create_flow(flow_name, flow_param, rga_param);
    if (!rga1) {
      assert(0);
      return -1;
    }
  }
  std::shared_ptr<easymedia::Flow> rga;
  do {
    std::string flow_name("filter");
    std::string filter_name("rkrga");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, filter_name);
    PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_ASYNCCOMMON);
    PixelFormat rga_out_pix_fmt = StringToPixFmt(IMAGE_RGB888);
    ImageInfo out_img_info = {rga_out_pix_fmt, npu_width, npu_height, npu_width,
                              npu_height};
    // if (!decoder && !rga_need_hold_input)
    //   PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, format);
    flow_param.append(easymedia::to_param_string(out_img_info, false));
    if (rga_need_hold_input)
      PARAM_STRING_APPEND_TO(flow_param, KEY_OUTPUT_HOLD_INPUT,
                             (int)easymedia::HoldInputMode::HOLD_INPUT);
    std::string rga_param;
    std::vector<ImageRect> v = {{0, 0, width, height},
                                {0, 0, npu_width, npu_height}};
    PARAM_STRING_APPEND(rga_param, KEY_BUFFER_RECT,
                        easymedia::TwoImageRectToString(v));
    rga = create_flow(flow_name, flow_param, rga_param);
    if (!rga) {
      assert(0);
      return -1;
    }
  } while (0);

  // rknn
  enum RK_NN_OUTPUT_TYPE type_of_npu_output = TYPE_INVALID_NPU_OUTPUT;
  std::shared_ptr<easymedia::Flow> rknn;
  if (!strncmp(model_name.c_str(), "rockx_", 6)) {
#if HAVE_ROCKX
    type_of_npu_output = TYPE_RK_ROCKX_OUTPUT;
    if (model_name != "rockx_face_gender_age" &&
        model_name != "rockx_face_detect") {
      fprintf(stderr, "TODO for %s\n", model_name.c_str());
      assert(0);
      return -1;
    }
    rknn = std::make_shared<RockxFlow>(model_name, render_result);
    if (!rknn || rknn->GetError()) {
      fprintf(stderr, "Fail to create rockx flow\n");
      return -1;
    }
#else
    fprintf(stderr, "rockx is not enable\n");
    return -1;
#endif
  } else if (!strncmp(model_name.c_str(), "rknn_", 5)) {
    assert(!model_path.empty());
    type_of_npu_output = TYPE_RK_NPU_OUTPUT;
    do {
      std::string flow_name("filter");
      std::string filter_name("rknn");
      std::string flow_param;
      PARAM_STRING_APPEND(flow_param, KEY_NAME, filter_name);
      PARAM_STRING_APPEND(flow_param, KEK_THREAD_SYNC_MODEL, KEY_ASYNCCOMMON);
      PARAM_STRING_APPEND(flow_param, KEY_INPUTDATATYPE, IMAGE_RGB888);
      if (render_result)
        PARAM_STRING_APPEND_TO(
            flow_param, KEY_OUTPUT_HOLD_INPUT,
            (int)easymedia::HoldInputMode::INHERIT_FORM_INPUT);
      std::string rknn_param;
      PARAM_STRING_APPEND(rknn_param, KEY_PATH, model_path);
      std::string str_tensor_type;
      std::string str_tensor_fmt;
      std::string str_want_float;
      if (model_name == "rknn_ssd") {
        str_tensor_type = NN_UINT8;
        str_tensor_fmt = KEY_NHWC;
        str_want_float = "1,1";
      } else {
        fprintf(stderr, "TODO for %s\n", model_name.c_str());
        assert(0);
        return -1;
      }
      PARAM_STRING_APPEND(rknn_param, KEY_TENSOR_TYPE, str_tensor_type);
      PARAM_STRING_APPEND(rknn_param, KEY_TENSOR_FMT, str_tensor_fmt);
      PARAM_STRING_APPEND(rknn_param, KEY_OUTPUT_WANT_FLOAT, str_want_float);
      rknn = create_flow(flow_name, flow_param, rknn_param);
      if (!rknn) {
        assert(0);
        return -1;
      }
    } while (0);
  }
  rga->AddDownFlow(rknn, 0, 0);

  // uvc
  auto uvc = std::make_shared<UVCJoinFlow>(
      type_of_npu_output, model_name, npu_width, npu_height, render_result);
  if (!uvc || uvc->GetError() || !uvc->Init()) {
    fprintf(stderr, "Fail to create uvc\n");
    return -1;
  }

  // render
  std::shared_ptr<SDLDrawFlow> render;
  if (render_result) {
    render = std::make_shared<SDLDrawFlow>(type_of_npu_output, model_name,
                                           npu_width, npu_height);
    if (!render || render->GetError()) {
      fprintf(stderr, "Fail to create sdl draw flow\n");
      return -1;
    }
  }

  if (render)
    rknn->AddDownFlow(render, 0, 0);
  else
    rknn->AddDownFlow(uvc, 0, 1);

  // finally, create v4l2 flow
  std::shared_ptr<easymedia::Flow> v4l2_flow;
  do {
    std::string flow_name("source_stream");
    std::string stream_name("v4l2_capture_stream");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, stream_name);
    std::string v4l2_param;
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_USE_LIBV4L2, 0);
    PARAM_STRING_APPEND(v4l2_param, KEY_DEVICE, v4l2_video_path);
    PARAM_STRING_APPEND(v4l2_param, KEY_V4L2_CAP_TYPE,
                        KEY_V4L2_C_TYPE(VIDEO_CAPTURE_MPLANE));
    PARAM_STRING_APPEND(v4l2_param, KEY_V4L2_MEM_TYPE,
                        KEY_V4L2_M_TYPE(MEMORY_DMABUF));
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_FRAMES, 8);
    PARAM_STRING_APPEND(v4l2_param, KEY_OUTPUTDATATYPE, format);
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_BUFFER_WIDTH, width);
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_BUFFER_HEIGHT, height);
    v4l2_flow = create_flow(flow_name, flow_param, v4l2_param);
    if (!v4l2_flow) {
      assert(0);
      return -1;
    }
    if (render_result) {
      rga0->AddDownFlow(uvc, 0, 0);
      render->AddDownFlow(rga0, 0, 0);
      rga1->AddDownFlow(rga, 0, 0);
      if (decoder) {
        decoder->AddDownFlow(rga1, 0, 0);
        v4l2_flow->AddDownFlow(decoder, 0, 0);
      } else {
        v4l2_flow->AddDownFlow(rga1, 0, 0);
      }
    } else {
      if (decoder) {
        decoder->AddDownFlow(rga0, 0, 0);
        if (rga_need_hold_input) {
          decoder->AddDownFlow(rga1, 0, 0);
          rga1->AddDownFlow(rga, 0, 0);
        } else {
          decoder->AddDownFlow(rga, 0, 0);
        }
        rga0->AddDownFlow(uvc, 0, 0);
        v4l2_flow->AddDownFlow(decoder, 0, 0);
      } else {
        if (rga_need_hold_input) {
          rga1->AddDownFlow(rga, 0, 0);
          v4l2_flow->AddDownFlow(rga1, 0, 0);
        } else {
          v4l2_flow->AddDownFlow(rga, 0, 0);
        }
        if (rga0) {
          rga0->AddDownFlow(uvc, 0, 0);
          v4l2_flow->AddDownFlow(rga0, 0, 0);
        } else {
          v4l2_flow->AddDownFlow(uvc, 0, 0);
        }
      }
    }
  } while (0);

#ifndef NDEBUG
  term_init();
  while (read_key() != 'q')
    easymedia::msleep(10);
  term_deinit();
#else
  while (true)
    easymedia::msleep(10);
#endif

#if 1
  if (render_result) {
    if (decoder) {
      v4l2_flow->RemoveDownFlow(decoder);
      decoder->RemoveDownFlow(rga1);
    } else {
      v4l2_flow->RemoveDownFlow(rga1);
    }
    rga1->RemoveDownFlow(rga);
    render->RemoveDownFlow(rga0);
    rga0->RemoveDownFlow(uvc);
  } else {
    if (decoder) {
      v4l2_flow->RemoveDownFlow(decoder);
      v4l2_flow.reset();
      if (rga_need_hold_input) {
        decoder->RemoveDownFlow(rga1);
        rga1->RemoveDownFlow(rga);
        rga1.reset();
      } else {
        decoder->RemoveDownFlow(rga);
      }
      decoder->RemoveDownFlow(rga0);
      decoder.reset();
      rga0->RemoveDownFlow(uvc);
      rga0.reset();
    } else {
      if (rga_need_hold_input) {
        v4l2_flow->RemoveDownFlow(rga1);
        rga1->RemoveDownFlow(rga);
        rga1.reset();
      } else {
        v4l2_flow->RemoveDownFlow(rga);
      }
      if (rga0) {
        rga0->RemoveDownFlow(uvc);
        v4l2_flow->RemoveDownFlow(rga0);
      } else {
        v4l2_flow->RemoveDownFlow(uvc);
      }
      v4l2_flow.reset();
    }
  }
  rga->RemoveDownFlow(rknn);
  rga.reset();
  if (render)
    rknn->RemoveDownFlow(render);
  else
    rknn->RemoveDownFlow(uvc);
  render.reset();
  uvc.reset();
  rknn.reset();
#endif
  camera_engine_rkisp_stop(rkisp_engine);

  return 0;
}

std::shared_ptr<easymedia::Flow> create_flow(const std::string &flow_name,
                                             const std::string &flow_param,
                                             const std::string &elem_param) {
  auto &&param = easymedia::JoinFlowParam(flow_param, 1, elem_param);
  auto ret = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
      flow_name.c_str(), param.c_str());
  if (!ret)
    fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
  return ret;
}

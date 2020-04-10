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

// this file run on 3399 to show npu result
// data path: uvc camera -> mpp -> sdl

// the npu outputs data inside at the marker of 0xFFE2 in jpeg

#ifdef NDEBUG
#undef NDEBUG
#endif
#ifndef DEBUG
#define DEBUG
#endif

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <easymedia/buffer.h>
#include <easymedia/decoder.h>
#include <easymedia/flow.h>
#include <easymedia/key_string.h>
#include <easymedia/media_config.h>
#include <easymedia/utils.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../npu_pp_output.h"

static std::shared_ptr<easymedia::Flow>
create_flow(const std::string &flow_name, const std::string &flow_param,
            const std::string &elem_param) {
  auto &&param = easymedia::JoinFlowParam(flow_param, 1, elem_param);
  auto ret = easymedia::REFLECTOR(Flow)::Create<easymedia::Flow>(
      flow_name.c_str(), param.c_str());
  if (!ret)
    fprintf(stderr, "Create flow %s failed\n", flow_name.c_str());
  return ret;
}

static bool do_extract(easymedia::Flow *f,
                       easymedia::MediaBufferVector &input_vector);
class UVCExtractFlow : public easymedia::Flow {
public:
  UVCExtractFlow();
  virtual ~UVCExtractFlow() { StopAllThread(); }

private:
  std::shared_ptr<easymedia::VideoDecoder> rkmpp_jpeg_dec;
  friend bool do_extract(easymedia::Flow *f,
                         easymedia::MediaBufferVector &input_vector);
};

UVCExtractFlow::UVCExtractFlow() {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(2);
  sm.fetch_block.push_back(true);
  sm.output_slots.push_back(0);
  sm.output_slots.push_back(1);
  sm.process = do_extract;
  if (!InstallSlotMap(sm, "uvc_extract", -1)) {
    fprintf(stderr, "Fail to InstallSlotMap, %s\n", "uvc_extract");
    SetError(-EINVAL);
    return;
  }
  std::string codec_name("rkmpp");
  std::string dec_param;
  PARAM_STRING_APPEND(dec_param, KEY_INPUTDATATYPE, IMAGE_JPEG);
  // set output data type work only for jpeg, but except 1808
  PARAM_STRING_APPEND(dec_param, KEY_OUTPUTDATATYPE, IMAGE_NV12);
  PARAM_STRING_APPEND_TO(dec_param, KEY_OUTPUT_TIMEOUT, 5000);
  rkmpp_jpeg_dec =
      easymedia::REFLECTOR(Decoder)::Create<easymedia::VideoDecoder>(
          codec_name.c_str(), dec_param.c_str());
  if (!rkmpp_jpeg_dec) {
    fprintf(stderr, "Fail to create rkmpp jpeg decoder\n");
    SetError(-EINVAL);
    return;
  }
}

#define JPEG_SECTION_MAX_LEN ((uint16_t)-1)

bool do_extract(easymedia::Flow *f,
                easymedia::MediaBufferVector &input_vector) {
  UVCExtractFlow *flow = (UVCExtractFlow *)f;
  auto input = input_vector[0];
  if (!input)
    return false;
  // 1. extract the npu output
  struct extra_jpeg_data *ejd = nullptr;
  uint8_t *buffer = static_cast<uint8_t *>(input->GetPtr());
  size_t buffer_size = input->GetValidSize();
  size_t pos = 0, size = 0;
  if (buffer_size < 4) {
    fprintf(stderr, "input buffer size is too small, impossible!\n");
    return false;
  }
  int a = (int)buffer[pos++];
  if (a != 0xFF || buffer[pos++] != 0xD8) {
    fprintf(stderr, "input is not jpeg, impossible!\n");
    return false;
  }
  for (;;) {
    int marker = 0;
    uint16_t itemlen;
    uint16_t ll, lh;
    if (pos > buffer_size - 1)
      break;
    for (a = 0; a <= 16 && pos < buffer_size - 1; a++) {
      marker = (int)buffer[pos++];
      if (marker != 0xff)
        break;
      if (a >= 16) {
        fprintf(stderr, "too many padding bytes\n");
        return false;
      }
    }
    if (marker == 0xD9 || marker == 0xDA)
      break;
    if (pos > buffer_size - 2)
      break;
    lh = (uint16_t)buffer[pos++];
    ll = (uint16_t)buffer[pos++];
    itemlen = (lh << 8) | ll;
    if (itemlen < 2) {
      fprintf(stderr, "invalid marker!\n");
      return false;
    }
    if (pos + itemlen - 2 > buffer_size) {
      fprintf(stderr, "Premature end of jpeg?\n");
      return false;
    }
    if (marker == 0xe2) {
      void *tmp = realloc(ejd, size + itemlen - 2);
      if (!tmp) {
        free(ejd);
        fprintf(stderr, "Not enough memory for size=%d\n",
                (int)(size + itemlen - 2));
        return false;
      }
      memcpy(((uint8_t *)tmp) + size, buffer + pos, itemlen - 2);
      ejd = (struct extra_jpeg_data *)tmp;
      size += (itemlen - 2);
    }
    pos += itemlen - 2;
  }
  if (!ejd) {
    fprintf(stderr, "no ffe2, input is not from npu!\n");
    return false;
  }
  std::shared_ptr<easymedia::MediaBuffer> npu_output;
  if (ejd->npu_output_size > 0) {
    // fprintf(stderr, "ejd->npu_output_size: %d\n", (int)ejd->npu_output_size);
    if (size != (sizeof(struct extra_jpeg_data) + ejd->npu_output_size)) {
      fprintf(stderr, "broken remote npu data!\n");
      return false;
    }
    auto npp = std::make_shared<NPUPostProcessOutput>(ejd);
    if (npp && npp->pp_output) {
      npu_output = std::make_shared<easymedia::MediaBuffer>();
      if (npu_output) {
        npu_output->SetUserData(npp);
        npu_output->SetPtr(npp.get());
        npu_output->SetValidSize(sizeof(NPUPostProcessOutput));
        npu_output->SetUSTimeStamp(ejd->npu_outputs_timestamp);
      }
    }
  }
  // 2. decode
  auto decoder = flow->rkmpp_jpeg_dec;
  std::shared_ptr<easymedia::MediaBuffer> img_output =
      std::make_shared<easymedia::ImageBuffer>();
  if (decoder->Process(input, img_output))
    img_output = nullptr;
  bool ret = true;
  if (!npu_output && !img_output)
    ret = false;
  if (img_output) {
    assert(ejd);
    img_output->SetUSTimeStamp(ejd->picture_timestamp);
    ret &= flow->SetOutput(img_output, 0);
  }
  if (npu_output)
    ret &= flow->SetOutput(npu_output, 1);
  return ret;
}

static bool do_compose_draw(easymedia::Flow *f,
                            easymedia::MediaBufferVector &input_vector);
class SDLComposeFlow : public easymedia::Flow {
public:
  SDLComposeFlow(int rotate_degree);
  virtual ~SDLComposeFlow() {
    StopAllThread();
    if (texture)
      SDL_DestroyTexture(texture);
    if (renderer)
      SDL_DestroyRenderer(renderer);
    if (window)
      SDL_DestroyWindow(window);
    SDL_Quit();
    fprintf(stderr, "sdl quit\n");
  }

private:
  int rotate;
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_RendererInfo renderer_info;
  SDL_Rect rect;
  bool prepared;
  std::shared_ptr<easymedia::MediaBuffer> last_npu_output;

  bool SDLPrepare();
  friend bool do_compose_draw(easymedia::Flow *f,
                              easymedia::MediaBufferVector &input_vector);
};

SDLComposeFlow::SDLComposeFlow(int rotate_degree)
    : rotate(rotate_degree), window(nullptr), renderer(nullptr),
      texture(nullptr), rect({0, 0, 0, 0}), prepared(false) {
  easymedia::SlotMap sm;
  sm.thread_model = easymedia::Model::ASYNCCOMMON;
  sm.mode_when_full = easymedia::InputMode::DROPFRONT;
  sm.input_slots.push_back(0);
  sm.input_maxcachenum.push_back(2);
  sm.fetch_block.push_back(true);
  sm.input_slots.push_back(1);
  sm.input_maxcachenum.push_back(1);
  sm.fetch_block.push_back(false);
  sm.process = do_compose_draw;
  if (!InstallSlotMap(sm, "sdl_compose_draw", -1)) {
    fprintf(stderr, "Fail to InstallSlotMap, %s\n", "sdl_compose_draw");
    SetError(-EINVAL);
    return;
  }
}

bool SDLComposeFlow::SDLPrepare() {
  SDL_LogSetPriority(SDL_LOG_CATEGORY_VIDEO, SDL_LOG_PRIORITY_VERBOSE);
  SDL_LogSetPriority(SDL_LOG_CATEGORY_RENDER, SDL_LOG_PRIORITY_VERBOSE);
  if (SDL_Init(SDL_INIT_VIDEO)) { // SDL_INIT_TIMER
    fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
    fprintf(stderr, "(Did you set the DISPLAY variable?)\n");
    return false;
  }
  SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
  SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
  int flags = SDL_WINDOW_SHOWN;
  flags |= SDL_WINDOW_FULLSCREEN;
  flags |= SDL_WINDOW_OPENGL;
  // flags |= SDL_WINDOW_BORDERLESS;
  flags |= SDL_WINDOW_RESIZABLE;
  window = SDL_CreateWindow("NPU UVC Demo", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, 1280, 720, flags);
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  if (!window) {
    fprintf(stderr, "Failed to create SDL window %s\n", SDL_GetError());
    return false;
  }
  renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!renderer) {
    fprintf(stderr,
            "Failed to initialize a hardware accelerated renderer: %s\n",
            SDL_GetError());
    return false;
  }
  if (!SDL_GetRendererInfo(renderer, &renderer_info))
    fprintf(stderr, "Initialized %s renderer\n", renderer_info.name);
  if (!renderer_info.num_texture_formats) {
    fprintf(stderr, "Broken SDL renderer %s\n", renderer_info.name);
    return false;
  }
  int displayIndex = SDL_GetWindowDisplayIndex(window);
  SDL_GetDisplayBounds(displayIndex, &rect);
  fprintf(stderr, "window display w,h: %d, %d\n", rect.w, rect.h);

  return true;
}

static const struct TextureFormatEntry {
  PixelFormat format;
  Uint32 texture_fmt;
} sdl_texture_format_map[] = {
    {PIX_FMT_RGB565, SDL_PIXELFORMAT_RGB565},
    {PIX_FMT_BGR565, SDL_PIXELFORMAT_BGR565},
    {PIX_FMT_RGB888, SDL_PIXELFORMAT_RGB24},
    {PIX_FMT_BGR888, SDL_PIXELFORMAT_BGR24},
    //{ AV_PIX_FMT_NE(RGB0, 0BGR), SDL_PIXELFORMAT_RGBX8888 },
    //{ AV_PIX_FMT_NE(BGR0, 0RGB), SDL_PIXELFORMAT_BGRX8888 },
    {PIX_FMT_ARGB8888, SDL_PIXELFORMAT_ARGB8888},
    {PIX_FMT_ABGR8888, SDL_PIXELFORMAT_ABGR8888},
    {PIX_FMT_YUV420P, SDL_PIXELFORMAT_IYUV},
    {PIX_FMT_NV12, SDL_PIXELFORMAT_NV12},
    {PIX_FMT_YUYV422, SDL_PIXELFORMAT_YUY2},
    {PIX_FMT_UYVY422, SDL_PIXELFORMAT_UYVY},
    {PIX_FMT_NONE, SDL_PIXELFORMAT_UNKNOWN},
};

static Uint32 get_sdl_pix_fmt(PixelFormat format) {
  for (size_t i = 0; i < ARRAY_ELEMS(sdl_texture_format_map) - 1; i++) {
    if (format == sdl_texture_format_map[i].format) {
      return sdl_texture_format_map[i].texture_fmt;
    }
  }
  return SDL_PIXELFORMAT_UNKNOWN;
}

static int realloc_texture(SDL_Texture **texture, SDL_Renderer *renderer,
                           Uint32 new_format, int new_width, int new_height,
                           SDL_BlendMode blendmode) {
  Uint32 format;
  int access, w, h;
  if (!*texture || SDL_QueryTexture(*texture, &format, &access, &w, &h) < 0 ||
      new_width != w || new_height != h || new_format != format) {
    if (*texture)
      SDL_DestroyTexture(*texture);
    if (!(*texture = SDL_CreateTexture(renderer, new_format,
                                       SDL_TEXTUREACCESS_STREAMING, new_width,
                                       new_height)))
      return -1;
    if (SDL_SetTextureBlendMode(*texture, blendmode) < 0)
      return -1;
    fprintf(stderr, "Created %dx%d texture with %s.\n", new_width, new_height,
            SDL_GetPixelFormatName(new_format));
  }
  return 0;
}

bool do_compose_draw(easymedia::Flow *f,
                     easymedia::MediaBufferVector &input_vector) {
  SDLComposeFlow *flow = (SDLComposeFlow *)f;
  if (!flow->prepared)
    flow->prepared = flow->SDLPrepare();
  if (!flow->prepared)
    return false;
  auto img_buf = input_vector[0];
  if (!img_buf || img_buf->GetType() != Type::Image)
    return false;
  auto img = std::static_pointer_cast<easymedia::ImageBuffer>(img_buf);
  Uint32 sdl_fmt = get_sdl_pix_fmt(img->GetPixelFormat());
  if (sdl_fmt == SDL_PIXELFORMAT_UNKNOWN) {
    fprintf(stderr, "unsupport pixel format: %d\n", img->GetPixelFormat());
    return false;
  }
  if (sdl_fmt != SDL_PIXELFORMAT_NV12) {
    fprintf(stderr, "not nv12? rk board should always send nv12!\n");
    return false;
  }
  auto &texture = flow->texture;
  if (realloc_texture(&texture, flow->renderer, sdl_fmt, img->GetVirWidth(),
                      img->GetVirHeight(), SDL_BLENDMODE_NONE)) {
    fprintf(stderr, "Failed to alloc renderer texture %s\n", SDL_GetError());
    return false;
  }
  // 1. draw picture
  int ret = SDL_UpdateTexture(texture, NULL, img->GetPtr(), img->GetVirWidth());
  static SDL_Rect dst_rect = {0, 0, 0, 0};
  if (!ret) {
    SDL_Rect src_rect = {0, 0, img->GetWidth(), img->GetHeight()};
    dst_rect = flow->rect;
    if (flow->rotate == 90 || flow->rotate == 270) {
      dst_rect.x = (dst_rect.w - dst_rect.h) / 2;
      dst_rect.y = (dst_rect.h - dst_rect.w) / 2;
      std::swap<int>(dst_rect.w, dst_rect.h);
    }
    if (src_rect.w * dst_rect.h > src_rect.h * dst_rect.w) {
      auto h = (dst_rect.w * src_rect.h) / src_rect.w;
      h &= ~1;
      dst_rect.y += (dst_rect.h - h) / 2;
      dst_rect.h = h;
    } else {
      auto w = (dst_rect.h * src_rect.w) / src_rect.h;
      w &= ~1;
      dst_rect.x += (dst_rect.w - w) / 2;
      dst_rect.w = w;
    }
    // fprintf(stderr, "src: %d,%d-%d,%d, dst: %d,%d-%d,%d\n", src_rect.x,
    //         src_rect.y, src_rect.w, src_rect.h, dst_rect.x, dst_rect.y,
    //         dst_rect.w, dst_rect.h);
    SDL_RenderCopyEx(flow->renderer, texture, &src_rect, &dst_rect,
                     flow->rotate, NULL, SDL_FLIP_NONE);
  }
  // 2. draw npu output
  auto npu_out_put = input_vector[1];
  if (npu_out_put) {
    flow->last_npu_output = npu_out_put;
  } else {
    if (flow->last_npu_output) {
      auto diff = (img_buf->GetUSTimeStamp() -
                   flow->last_npu_output->GetUSTimeStamp()) /
                  1000;
      if (diff >= 0 && diff < 500)
        npu_out_put = flow->last_npu_output;
    }
  }
  if (npu_out_put && dst_rect.w > 0 && dst_rect.h > 0) {
    auto npo = (NPUPostProcessOutput *)npu_out_put->GetPtr();
    (npo->pp_func)(flow->renderer, dst_rect, flow->rect, flow->rotate, npo,
                   nullptr, sdl_fmt);
  }
  SDL_RenderPresent(flow->renderer);
  return false;
}

#include "term_help.h"

static char optstr[] = "?i:w:h:r:";

int main(int argc, char **argv) {
  int c;
  std::string v4l2_video_path;
  int width = 1280, height = 720;
  int rotate = 0;
  opterr = 1;
  while ((c = getopt(argc, argv, optstr)) != -1) {
    switch (c) {
    case 'i':
      v4l2_video_path = optarg;
      printf("input path: %s\n", v4l2_video_path.c_str());
      break;
    case 'w':
      width = atoi(optarg);
      break;
    case 'h':
      height = atoi(optarg);
      break;
    case 'r':
      rotate = atoi(optarg);
      if (rotate != 0 && rotate != 90 && rotate != 180 && rotate != 270) {
        fprintf(stderr, "TODO: rotate is not 0/90/180/270\n");
        rotate = 0;
      }
      break;
    case '?':
    default:
      printf("help:\n\t-i: v4l2 capture device path\n");
      printf("\t-w: v4l2 capture width\n");
      printf("\t-h: v4l2 capture height\n");
      printf("\t-r: display rotate\n");
      printf("\nusage example: \n");
      printf("rk_npu_uvc_host -i /dev/video0 -w 1280 -h 720 -r 270\n");
      exit(0);
    }
  }
  assert(!v4l2_video_path.empty());

  auto extract_flow = std::make_shared<UVCExtractFlow>();
  if (!extract_flow || extract_flow->GetError()) {
    fprintf(stderr, "Fail to create uvc extract flow\n");
    return -1;
  }
  auto render_flow = std::make_shared<SDLComposeFlow>(rotate);
  if (!render_flow || render_flow->GetError()) {
    fprintf(stderr, "Fail to create sdl compose draw flow\n");
    return -1;
  }
  extract_flow->AddDownFlow(render_flow, 0, 0);
  extract_flow->AddDownFlow(render_flow, 1, 1);
  // finally, create v4l2 flow
  std::shared_ptr<easymedia::Flow> v4l2_flow;
  do {
    std::string flow_name("source_stream");
    std::string stream_name("v4l2_capture_stream");
    std::string flow_param;
    PARAM_STRING_APPEND(flow_param, KEY_NAME, stream_name);
    std::string v4l2_param;
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_USE_LIBV4L2, 1);
    PARAM_STRING_APPEND(v4l2_param, KEY_DEVICE, v4l2_video_path);
    PARAM_STRING_APPEND(v4l2_param, KEY_V4L2_CAP_TYPE,
                        KEY_V4L2_C_TYPE(VIDEO_CAPTURE));
    PARAM_STRING_APPEND(v4l2_param, KEY_V4L2_MEM_TYPE,
                        KEY_V4L2_M_TYPE(MEMORY_MMAP));
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_FRAMES, 4);
    // always jpeg
    PARAM_STRING_APPEND(v4l2_param, KEY_OUTPUTDATATYPE, IMAGE_JPEG);
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_BUFFER_WIDTH, width);
    PARAM_STRING_APPEND_TO(v4l2_param, KEY_BUFFER_HEIGHT, height);
    v4l2_flow = create_flow(flow_name, flow_param, v4l2_param);
    if (!v4l2_flow) {
      assert(0);
      return -1;
    }
  } while (0);
  v4l2_flow->AddDownFlow(extract_flow, 0, 0);

#ifndef NDEBUG
  term_init();
  while (read_key() != 'q')
    easymedia::msleep(10);
  term_deinit();
#else
  while (true)
    easymedia::msleep(10);
#endif
  fprintf(stderr, "quit loop\n");
  v4l2_flow->RemoveDownFlow(extract_flow);
  v4l2_flow.reset();
  extract_flow->RemoveDownFlow(render_flow);
  extract_flow.reset();
  render_flow.reset();

  return 0;
}

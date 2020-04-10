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

#include "npu_pp_output.h"

#include <assert.h>

#include "rknn_ssd.h"
#include "rockx_draw.h"

NPUPostProcessOutput::NPUPostProcessOutput(struct extra_jpeg_data *ejd)
    : pp_output(nullptr), count(ejd->npu_outputs_num), pp_func(nullptr),
      npuwh(ejd->npuwh) {
  switch (ejd->npu_output_type) {
  case TYPE_RK_NPU_OUTPUT: {
    size_t pos = 0;
    struct aligned_npu_output ano[ejd->npu_outputs_num];
    uint8_t *bufs[ejd->npu_outputs_num] = {nullptr};
    for (size_t i = 0; i < ejd->npu_outputs_num; i++) {
      auto pano = (struct aligned_npu_output *)(ejd->outputs + pos);
      ano[i] = *pano;
      pos += sizeof(*pano);
      bufs[i] = ejd->outputs + pos;
      pos += pano->size;
    }
    assert(pos == ejd->npu_output_size);
    if (!strcmp((const char *)ejd->model_identifier, "rknn_ssd")) {
      pp_output = malloc(sizeof(NPU_UVC_SSD_DEMO::detect_result_group_t));
      if (!pp_output) {
        assert(0);
        goto fail;
      }
      auto group = (NPU_UVC_SSD_DEMO::detect_result_group_t *)pp_output;
      int ret = NPU_UVC_SSD_DEMO::postProcessSSD(
          (float *)(bufs[1]), (float *)(bufs[0]), ejd->npuwh.width,
          ejd->npuwh.height, group);
      if (ret) {
        fprintf(stderr, "Fail to postProcessSSD\n");
        goto fail;
      }
      npuwh = ejd->npuwh;
      pp_func = NPU_UVC_SSD_DEMO::SSDDraw;
    } else {
      fprintf(stderr, "TODO %s: %d\n", __FUNCTION__, __LINE__);
      goto fail;
    }
  } break;
  case TYPE_RK_ROCKX_OUTPUT: {
    assert(sizeof(float) == 4);
    pp_output = malloc(ejd->npu_output_size);
    if (!pp_output) {
      assert(0);
      goto fail;
    }
    memcpy(pp_output, ejd->outputs, ejd->npu_output_size);
    npuwh = ejd->npuwh;
    auto model_iden = (const char *)ejd->model_identifier;
    if (!strcmp(model_iden, "rockx_face_gender_age")) {
      pp_func = NPU_UVC_ROCKX_DEMO::RockxFaceGenderAgeDraw;
    } else if (!strcmp(model_iden, "rockx_face_detect")) {
      pp_func = NPU_UVC_ROCKX_DEMO::RockxFaceDetectDraw;
    } else {
      fprintf(stderr, "TODO %s: %d\n", __FUNCTION__, __LINE__);
      goto fail;
    }
  } break;
  default:
    fprintf(stderr, "unimplemented rk nn output type: %d\n",
            ejd->npu_output_type);
    break;
  }
  return;
fail:
  if (pp_output) {
    free(pp_output);
    pp_output = nullptr;
  }
}

// rotate by center (coor_rect.w/2, coor_rect.h/2)
SDL_Rect transform(const SDL_Rect &src_rect, const SDL_Rect &coor_rect,
                   int rotate) {
  SDL_Rect rect = src_rect;
  switch (rotate) {
  case 0:
    break;
  case 90:
    rect.x = -src_rect.y - src_rect.h + coor_rect.h / 2 + coor_rect.w / 2;
    rect.y = src_rect.x - coor_rect.w / 2 + coor_rect.h / 2;
    rect.w = src_rect.h;
    rect.h = src_rect.w;
    break;
  case 180:
  case 270:
  default:
    fprintf(stderr, "TODO: rotate=%d\n", rotate);
    break;
  }
  return rect;
}

#define FONT_FILE_PATH "/usr/lib/fonts/DejaVuSansMono.ttf"
static SDL_Color white = {0xFF, 0xFF, 0xFF, 0x00};
SDL_Color red = {0x00, 0x00, 0xFF, 0xFF};
// static SDL_Color title_color = {0x06, 0xEB, 0xFF, 0xFF};
class SDLTTF {
public:
  SDLTTF() {
    if (TTF_Init() < 0)
      fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
  }
  ~SDLTTF() { TTF_Quit(); }
};

static SDLTTF sdl_ttf;

SDLFont::SDLFont(SDL_Color forecol, int ptsize)
    : fore_col(forecol), back_col(white), renderstyle(TTF_STYLE_NORMAL),
      rendertype(RENDER_UTF8), pt_size(ptsize), font(NULL) {
  font = TTF_OpenFont(FONT_FILE_PATH, ptsize);
  if (font == NULL) {
    fprintf(stderr, "hehe Couldn't load %d pt font from %s: %s\n", ptsize,
            FONT_FILE_PATH, SDL_GetError());
    return;
  }
  TTF_SetFontStyle(font, renderstyle);
}

SDLFont::~SDLFont() {
  if (font) {
    TTF_CloseFont(font);
  }
}

SDL_Surface *SDLFont::DrawString(char *str, int str_length) {
  SDL_Surface *text = NULL;
  switch (rendertype) {
  case RENDER_LATIN1:
    text = TTF_RenderText_Blended(font, str, fore_col);
    break;

  case RENDER_UTF8:
    text = TTF_RenderUTF8_Blended(font, str, fore_col);
    break;

  case RENDER_UNICODE: {
    Uint16 *unicode_text = (Uint16 *)malloc(2 * str_length + 1);
    if (!unicode_text)
      break;
    int index;
    for (index = 0; (str[0] || str[1]); ++index) {
      unicode_text[index] = ((Uint8 *)str)[0];
      unicode_text[index] <<= 8;
      unicode_text[index] |= ((Uint8 *)str)[1];
      str += 2;
    }
    unicode_text[index] = 0;
    text = TTF_RenderUNICODE_Blended(font, unicode_text, fore_col);
    free(unicode_text);
  } break;
  default:
    /* This shouldn't happen */
    break;
  }
  return text;
}

// static int power_of_two(int input)
// {
//     int value = 1;

//     while ( value < input ) {
//         value <<= 1;
//     }
//     return value;
// }

SDL_Surface *SDLFont::GetFontPicture(char *str, int str_length, int bpp, int *w,
                                     int *h) {
  SDL_Surface *image;
  SDL_Rect area;
  // Uint8  saved_alpha;
  // SDL_BlendMode saved_mode;
  if (str_length <= 0)
    return NULL;
  SDL_Surface *text = DrawString(str, str_length);
  if (!text) {
    fprintf(stderr, "draw %s to picture failed\n", str);
    return NULL;
  }
  *w = text->w; // power_of_two(text->w);
  *h = text->h; // power_of_two(text->h);
  if (bpp == 32)
    return text;
  image = SDL_CreateRGBSurfaceWithFormat(SDL_SWSURFACE, *w, *h, bpp,
                                         SDL_PIXELFORMAT_RGB24);
  if (image == NULL) {
    fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
    return NULL;
  }
  /* Save the alpha blending attributes */
  // SDL_GetSurfaceAlphaMod(text, &saved_alpha);
  // SDL_SetSurfaceAlphaMod(text, 0xFF);
  // SDL_GetSurfaceBlendMode(text, &saved_mode);
  // SDL_SetSurfaceBlendMode(text, SDL_BLENDMODE_NONE);
  /* Copy the text into the GL texture image */
  area.x = 0;
  area.y = 0;
  area.w = text->w;
  area.h = text->h;
  SDL_BlitSurface(text, &area, image, &area);
  /* Restore the alpha blending attributes */
  // SDL_SetSurfaceAlphaMod(text, saved_alpha);
  // SDL_SetSurfaceBlendMode(text, saved_mode);
  SDL_FreeSurface(text);
  return image;
}

SDL_Texture *load_texture(SDL_Surface *sur, SDL_Renderer *render,
                          SDL_Rect *texture_dimensions) {
  SDL_Texture *texture = SDL_CreateTextureFromSurface(render, sur);
  assert(texture);
  Uint32 pixelFormat;
  int access;
  texture_dimensions->x = 0;
  texture_dimensions->y = 0;
  SDL_QueryTexture(texture, &pixelFormat, &access, &texture_dimensions->w,
                   &texture_dimensions->h);
  return texture;
}

int draw_rect(SDL_Renderer *renderer, const SDL_Rect *rect, void *buffer,
              Uint32 sdl_fmt) {
  int status = SDL_RenderDrawRect(renderer, rect);
  if (status && sdl_fmt == SDL_PIXELFORMAT_RGB24) {
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    SDL_Rect viewport;
    SDL_RenderGetViewport(renderer, &viewport);
    uint8_t *ptr = (uint8_t *)buffer + (rect->x + rect->y * viewport.w) * 3;
    uint8_t *p = ptr;

    if (rect->x < 0 || rect->x > viewport.w ||
	rect->y < 0 || rect->y > viewport.h ||
	(rect->x + rect->w  >= viewport.w) ||
	(rect->y + rect->h  >= viewport.h))
	return 0;

    for (int i = 0; i < rect->w; i++) {
      *p++ = r;
      *p++ = g;
      *p++ = b;
    }
    p = ptr;
    for (int i = 0; i < rect->h; i++) {
      *p = r;
      *(p + 1) = g;
      *(p + 2) = b;
      p += viewport.w * 3;
    }
    for (int i = 0; i < rect->w; i++) {
      *p++ = r;
      *p++ = g;
      *p++ = b;
    }
    p = ptr + rect->w * 3;
    for (int i = 0; i < rect->h; i++) {
      *p = r;
      *(p + 1) = g;
      *(p + 2) = b;
      p += viewport.w * 3;
    }
    status = 0;
  }
  return status;
}

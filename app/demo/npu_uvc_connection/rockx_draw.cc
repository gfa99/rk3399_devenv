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

#include "rockx_draw.h"
#include <assert.h>

#include "npu_pp_output.h"

namespace NPU_UVC_ROCKX_DEMO {

static SDLFont fga_sdl_font(red, 40);
bool RockxFaceGenderAgeDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
                            const SDL_Rect &coor_rect, int rotate,
                            NPUPostProcessOutput *output, void *buffer,
                            Uint32 sdl_fmt) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0xFF, 0x10, 0xEB, 0xFF);
  int npu_w = output->npuwh.width;
  int npu_h = output->npuwh.height;
  char gender_age[32] = {0};
  static const char *man = "Gender: Man, Age:";
  static const char *women = "Gender: Women, Age:";
  auto face = (struct aligned_rockx_face_gender_age *)(output->pp_output);
  for (uint32_t i = 0; i < output->count; i++) {
    assert(sizeof(float) == 4);
    float score;
    auto fga = face + i;
    memcpy(&score, fga->score, 4);
    if (score < 0.85)
      continue;
    snprintf(gender_age, sizeof(gender_age), "%s %d", fga->gender ? man : women,
             (int)fga->age);
    int x1 = fga->left;
    int y1 = fga->top;
    int x2 = fga->right;
    int y2 = fga->bottom;
    SDL_Rect rect = {x1 * render_rect.w / npu_w + render_rect.x,
                     y1 * render_rect.h / npu_h + render_rect.y,
                     (x2 - x1) * render_rect.w / npu_w,
                     (y2 - y1) * render_rect.h / npu_h};
    rect = transform(rect, coor_rect, rotate);
    int status = draw_rect(renderer, &rect, buffer, sdl_fmt);
    if (status)
      fprintf(stderr, "draw rect status: %d <%s>\n", status, SDL_GetError());
    int fontw = 0, fonth = 0;
    SDL_Surface *str = fga_sdl_font.GetFontPicture(
        gender_age, strlen(gender_age), 32, &fontw, &fonth);
    if (str) {
      SDL_Rect texture_dimension;
      SDL_Texture *texture = load_texture(str, renderer, &texture_dimension);
      SDL_FreeSurface(str);
      SDL_Rect dst_dimension;
      switch (rotate) {
      case 0:
        dst_dimension.x = rect.x;
        dst_dimension.y = rect.y - 36;
        break;
      case 90:
        dst_dimension.x =
            rect.x + rect.w + texture_dimension.h / 2 - texture_dimension.w / 2;
        dst_dimension.y = rect.y + rect.h / 2 - texture_dimension.h / 2;
        break;
      default:
        fprintf(stderr, "TODO: rotate=%d\n", rotate);
        dst_dimension.x = rect.x;
        dst_dimension.y = rect.y;
        break;
      }
      dst_dimension.w = texture_dimension.w;
      dst_dimension.h = texture_dimension.h;
      SDL_RenderCopyEx(renderer, texture, &texture_dimension, &dst_dimension,
                       rotate, NULL, SDL_FLIP_NONE);
      SDL_DestroyTexture(texture);
    }
  }

  return true;
}

bool RockxFaceDetectDraw(SDL_Renderer *renderer, const SDL_Rect &render_rect,
                         const SDL_Rect &coor_rect, int rotate,
                         NPUPostProcessOutput *output, void *buffer,
                         Uint32 sdl_fmt) {
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
  SDL_SetRenderDrawColor(renderer, 0xFF, 0x10, 0xEB, 0xFF);
  int npu_w = output->npuwh.width;
  int npu_h = output->npuwh.height;
  auto fr = (struct aligned_rockx_face_rect *)(output->pp_output);
  for (uint32_t i = 0; i < output->count; i++) {
    assert(sizeof(float) == 4);
    float score;
    auto face = fr + i;
    memcpy(&score, face->score, 4);
    if (score < 0.8f)
      continue;
    int x1 = face->left;
    int y1 = face->top;
    int x2 = face->right;
    int y2 = face->bottom;
    SDL_Rect rect = {x1 * render_rect.w / npu_w + render_rect.x,
                     y1 * render_rect.h / npu_h + render_rect.y,
                     (x2 - x1) * render_rect.w / npu_w,
                     (y2 - y1) * render_rect.h / npu_h};
    rect = transform(rect, coor_rect, rotate);
    int status = draw_rect(renderer, &rect, buffer, sdl_fmt);
    if (status)
      fprintf(stderr, "draw rect status: %d <%s>\n", status, SDL_GetError());
  }

  return true;
}

} // namespace NPU_UVC_ROCKX_DEMO
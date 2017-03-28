/*
 * Copyright (C) Fuzhou Rockchip Electronics Co.Ltd
 * Author:Mark Yao <mark.yao@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <config.h>
#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <fdtdec.h>
#include <fdt_support.h>
#include <resource.h>
#include <asm/arch/rkplat.h>
#include <asm/unaligned.h>
#include <linux/list.h>

#include "rockchip_vop.h"
#include "rockchip_vop_reg.h"

#define VOP_REG(off, _mask, s) \
		{.offset = off, \
		 .mask = _mask, \
		 .shift = s, \
		 .write_mask = false,}

#define VOP_REG_MASK(off, _mask, s) \
		{.offset = off, \
		 .mask = _mask, \
		 .shift = s, \
		 .write_mask = true,}

static const struct vop_scl_regs rk3066_win_scl = {
	.scale_yrgb_x = VOP_REG(RK3036_WIN0_SCL_FACTOR_YRGB, 0xffff, 0x0),
	.scale_yrgb_y = VOP_REG(RK3036_WIN0_SCL_FACTOR_YRGB, 0xffff, 16),
	.scale_cbcr_x = VOP_REG(RK3036_WIN0_SCL_FACTOR_CBR, 0xffff, 0x0),
	.scale_cbcr_y = VOP_REG(RK3036_WIN0_SCL_FACTOR_CBR, 0xffff, 16),
};

static const struct vop_win rk3036_win0_data = {
	.scl = &rk3066_win_scl,
	.enable = VOP_REG(RK3036_SYS_CTRL, 0x1, 0),
	.format = VOP_REG(RK3036_SYS_CTRL, 0x7, 3),
	.rb_swap = VOP_REG(RK3036_SYS_CTRL, 0x1, 15),
	.act_info = VOP_REG(RK3036_WIN0_ACT_INFO, 0x1fff1fff, 0),
	.dsp_info = VOP_REG(RK3036_WIN0_DSP_INFO, 0x0fff0fff, 0),
	.dsp_st = VOP_REG(RK3036_WIN0_DSP_ST, 0x1fff1fff, 0),
	.yrgb_mst = VOP_REG(RK3036_WIN0_YRGB_MST, 0xffffffff, 0),
	.uv_mst = VOP_REG(RK3036_WIN0_CBR_MST, 0xffffffff, 0),
	.yrgb_vir = VOP_REG(RK3036_WIN0_VIR, 0xffff, 0),
};

static const struct vop_ctrl rk3036_ctrl_data = {
	.standby = VOP_REG(RK3036_SYS_CTRL, 0x1, 30),
	.out_mode = VOP_REG(RK3036_DSP_CTRL0, 0xf, 0),
	.pin_pol = VOP_REG(RK3036_DSP_CTRL0, 0xf, 4),
	.dsp_layer_sel = VOP_REG(RK3036_DSP_CTRL0, 0x1, 8),
	.htotal_pw = VOP_REG(RK3036_DSP_HTOTAL_HS_END, 0x1fff1fff, 0),
	.hact_st_end = VOP_REG(RK3036_DSP_HACT_ST_END, 0x1fff1fff, 0),
	.vtotal_pw = VOP_REG(RK3036_DSP_VTOTAL_VS_END, 0x1fff1fff, 0),
	.vact_st_end = VOP_REG(RK3036_DSP_VACT_ST_END, 0x1fff1fff, 0),
	.line_flag_num[0] = VOP_REG(RK3036_INT_STATUS, 0xfff, 12),
	.cfg_done = VOP_REG(RK3036_REG_CFG_DONE, 0x1, 0),
};

static const struct vop_reg_data rk3036_vop_init_reg_table[] = {
	{RK3036_DSP_CTRL1, 0x00000000},
};

static const struct vop_data rk3036_vop = {
	.init_table = rk3036_vop_init_reg_table,
	.table_size = ARRAY_SIZE(rk3036_vop_init_reg_table),
	.ctrl = &rk3036_ctrl_data,
	.win = &rk3036_win0_data,
	.reg_len = RK3036_BCSH_H * 4,
};

static const struct vop_scl_extension rk3288_win_full_scl_ext = {
	.cbcr_vsd_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 31),
	.cbcr_vsu_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 30),
	.cbcr_hsd_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 28),
	.cbcr_ver_scl_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 26),
	.cbcr_hor_scl_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 24),
	.yrgb_vsd_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 23),
	.yrgb_vsu_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 22),
	.yrgb_hsd_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 20),
	.yrgb_ver_scl_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 18),
	.yrgb_hor_scl_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 16),
	.line_load_mode = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 15),
	.cbcr_axi_gather_num = VOP_REG(RK3288_WIN0_CTRL1, 0x7, 12),
	.yrgb_axi_gather_num = VOP_REG(RK3288_WIN0_CTRL1, 0xf, 8),
	.vsd_cbcr_gt2 = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 7),
	.vsd_cbcr_gt4 = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 6),
	.vsd_yrgb_gt2 = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 5),
	.vsd_yrgb_gt4 = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 4),
	.bic_coe_sel = VOP_REG(RK3288_WIN0_CTRL1, 0x3, 2),
	.cbcr_axi_gather_en = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 1),
	.yrgb_axi_gather_en = VOP_REG(RK3288_WIN0_CTRL1, 0x1, 0),
	.lb_mode = VOP_REG(RK3288_WIN0_CTRL0, 0x7, 5),
};

static const struct vop_scl_regs rk3288_win_full_scl = {
	.ext = &rk3288_win_full_scl_ext,
	.scale_yrgb_x = VOP_REG(RK3288_WIN0_SCL_FACTOR_YRGB, 0xffff, 0x0),
	.scale_yrgb_y = VOP_REG(RK3288_WIN0_SCL_FACTOR_YRGB, 0xffff, 16),
	.scale_cbcr_x = VOP_REG(RK3288_WIN0_SCL_FACTOR_CBR, 0xffff, 0x0),
	.scale_cbcr_y = VOP_REG(RK3288_WIN0_SCL_FACTOR_CBR, 0xffff, 16),
};

static const struct vop_win rk3288_win01_data = {
	.scl = &rk3288_win_full_scl,
	.enable = VOP_REG(RK3288_WIN0_CTRL0, 0x1, 0),
	.format = VOP_REG(RK3288_WIN0_CTRL0, 0x7, 1),
	.rb_swap = VOP_REG(RK3288_WIN0_CTRL0, 0x1, 12),
	.ymirror = VOP_REG(RK3288_WIN0_CTRL0, 0x1, 22),
	.act_info = VOP_REG(RK3288_WIN0_ACT_INFO, 0x1fff1fff, 0),
	.dsp_info = VOP_REG(RK3288_WIN0_DSP_INFO, 0x0fff0fff, 0),
	.dsp_st = VOP_REG(RK3288_WIN0_DSP_ST, 0x1fff1fff, 0),
	.yrgb_mst = VOP_REG(RK3288_WIN0_YRGB_MST, 0xffffffff, 0),
	.uv_mst = VOP_REG(RK3288_WIN0_CBR_MST, 0xffffffff, 0),
	.yrgb_vir = VOP_REG(RK3288_WIN0_VIR, 0x3fff, 0),
	.uv_vir = VOP_REG(RK3288_WIN0_VIR, 0x3fff, 16),
	.src_alpha_ctl = VOP_REG(RK3288_WIN0_SRC_ALPHA_CTRL, 0xff, 0),
	.dst_alpha_ctl = VOP_REG(RK3288_WIN0_DST_ALPHA_CTRL, 0xff, 0),
};

static const struct vop_ctrl rk3288_ctrl_data = {
	.standby = VOP_REG(RK3288_SYS_CTRL, 0x1, 22),
	.dsp_layer_sel = VOP_REG(RK3288_DSP_CTRL1, 0xff, 8),
	.gate_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 23),
	.mmu_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 20),
	.rgb_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 12),
	.hdmi_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 13),
	.edp_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 14),
	.mipi_en = VOP_REG(RK3288_SYS_CTRL, 0x1, 15),
	.dither_down = VOP_REG(RK3288_DSP_CTRL1, 0xf, 1),
	.dither_up = VOP_REG(RK3288_DSP_CTRL1, 0x1, 6),
	.data_blank = VOP_REG(RK3288_DSP_CTRL0, 0x1, 19),
	.out_mode = VOP_REG(RK3288_DSP_CTRL0, 0xf, 0),
	.pin_pol = VOP_REG(RK3288_DSP_CTRL0, 0xf, 4),
	.htotal_pw = VOP_REG(RK3288_DSP_HTOTAL_HS_END, 0x1fff1fff, 0),
	.hact_st_end = VOP_REG(RK3288_DSP_HACT_ST_END, 0x1fff1fff, 0),
	.vtotal_pw = VOP_REG(RK3288_DSP_VTOTAL_VS_END, 0x1fff1fff, 0),
	.vact_st_end = VOP_REG(RK3288_DSP_VACT_ST_END, 0x1fff1fff, 0),
	.hpost_st_end = VOP_REG(RK3288_POST_DSP_HACT_INFO, 0x1fff1fff, 0),
	.vpost_st_end = VOP_REG(RK3288_POST_DSP_VACT_INFO, 0x1fff1fff, 0),
	.line_flag_num[0] = VOP_REG(RK3288_INTR_CTRL0, 0x1fff, 12),
	.cfg_done = VOP_REG(RK3288_REG_CFG_DONE, 0x1, 0),
};

static const struct vop_reg_data rk3288_init_reg_table[] = {
	{RK3288_SYS_CTRL, 0x00c00000},
	{RK3288_DSP_CTRL0, 0x00000000},
	{RK3288_WIN0_CTRL0, 0x00000080},
	{RK3288_WIN1_CTRL0, 0x00000080},
	/*
	 * Bit[0] is win2/3 gate en bit, there is no power consume with this
	 * bit enable. the bit's function similar with area plane enable bit,
	 * So default enable this bit, then We can control win2/3 area plane
	 * with its enable bit.
	 */
	{RK3288_WIN2_CTRL0, 0x00000001},
	{RK3288_WIN3_CTRL0, 0x00000001},
};

static const struct vop_data rk3288_vop = {
	.init_table = rk3288_init_reg_table,
	.table_size = ARRAY_SIZE(rk3288_init_reg_table),
	.ctrl = &rk3288_ctrl_data,
	.win = &rk3288_win01_data,
	.reg_len = RK3288_DSP_VACT_ST_END_F1 * 4,
};

static const struct vop_ctrl rk3399_ctrl_data = {
	.standby = VOP_REG(RK3399_SYS_CTRL, 0x1, 22),
	.gate_en = VOP_REG(RK3399_SYS_CTRL, 0x1, 23),
	.rgb_en = VOP_REG(RK3399_SYS_CTRL, 0x1, 12),
	.hdmi_en = VOP_REG(RK3399_SYS_CTRL, 0x1, 13),
	.edp_en = VOP_REG(RK3399_SYS_CTRL, 0x1, 14),
	.mipi_en = VOP_REG(RK3399_SYS_CTRL, 0x1, 15),
	.dsp_layer_sel = VOP_REG(RK3399_DSP_CTRL1, 0xff, 8),
	.dither_down = VOP_REG(RK3399_DSP_CTRL1, 0xf, 1),
	.dither_up = VOP_REG(RK3399_DSP_CTRL1, 0x1, 6),
	.data_blank = VOP_REG(RK3399_DSP_CTRL0, 0x1, 19),
	.out_mode = VOP_REG(RK3399_DSP_CTRL0, 0xf, 0),
	.rgb_pin_pol = VOP_REG(RK3399_DSP_CTRL1, 0xf, 16),
	.hdmi_pin_pol = VOP_REG(RK3399_DSP_CTRL1, 0xf, 20),
	.edp_pin_pol = VOP_REG(RK3399_DSP_CTRL1, 0xf, 24),
	.mipi_pin_pol = VOP_REG(RK3399_DSP_CTRL1, 0xf, 28),
	.htotal_pw = VOP_REG(RK3399_DSP_HTOTAL_HS_END, 0x1fff1fff, 0),
	.hact_st_end = VOP_REG(RK3399_DSP_HACT_ST_END, 0x1fff1fff, 0),
	.vtotal_pw = VOP_REG(RK3399_DSP_VTOTAL_VS_END, 0x1fff1fff, 0),
	.vact_st_end = VOP_REG(RK3399_DSP_VACT_ST_END, 0x1fff1fff, 0),
	.hpost_st_end = VOP_REG(RK3399_POST_DSP_HACT_INFO, 0x1fff1fff, 0),
	.vpost_st_end = VOP_REG(RK3399_POST_DSP_VACT_INFO, 0x1fff1fff, 0),
	.line_flag_num[0] = VOP_REG(RK3399_LINE_FLAG, 0xffff, 0),
	.line_flag_num[1] = VOP_REG(RK3399_LINE_FLAG, 0xffff, 16),

	.cfg_done = VOP_REG_MASK(RK3399_REG_CFG_DONE, 0x1, 0),
};

static const struct vop_reg_data rk3399_init_reg_table[] = {
	{RK3399_SYS_CTRL, 0x2000f800},
	{RK3399_DSP_CTRL0, 0x00000000},
	{RK3399_DSP_BG, 0x00000000},
	{RK3399_WIN0_CTRL0, 0x00000080},
	{RK3399_WIN1_CTRL0, 0x00000080},
	/*
	 * Bit[0] is win2/3 gate en bit, there is no power consume with this
	 * bit enable. the bit's function similar with area plane enable bit,
	 * So default enable this bit, then We can control win2/3 area plane
	 * with its enable bit.
	 */
	{RK3399_WIN2_CTRL0, 0x00000001},
	{RK3399_WIN3_CTRL0, 0x00000001},
};

const struct vop_data rk3399_vop = {
	.init_table = rk3399_init_reg_table,
	.table_size = ARRAY_SIZE(rk3399_init_reg_table),
	.ctrl = &rk3399_ctrl_data,
	/*
	 * rk3399 vop big windows register layout is same as rk3288.
	 */
	.win = &rk3288_win01_data,
	.reg_len = RK3399_DSP_VACT_ST_END_F1 * 4,
};

const void *rk3036_vop_data = &rk3036_vop;
const void *rk3288_vop_data = &rk3288_vop;
const void *rk3399_vop_data = &rk3399_vop;

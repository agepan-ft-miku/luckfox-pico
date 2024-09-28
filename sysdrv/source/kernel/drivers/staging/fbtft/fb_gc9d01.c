// SPDX-License-Identifier: GPL-2.0+
/*
 * FB driver for the GC9D01 LCD display controller
 *
 * Copyright (C) 2024 agepan_ft_miku
 * Based on fb_ili9341.c by Christian Vogelgsang
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <video/mipi_display.h>

#include "fbtft.h"

#define DRVNAME		"fb_gc9d01"
#define WIDTH		160
#define HEIGHT		160
#define TXBUFLEN	(4 * PAGE_SIZE)
#define DEFAULT_GAMMA	"1F 1A 18 0A 0F 06 45 87 32 0A 07 02 07 05 00\n" \
			"00 25 27 05 10 09 3A 78 4D 05 18 0D 38 3A 1F"

static int init_display(struct fbtft_par *par)
{
	par->fbtftops.reset(par);

	write_reg(par, MIPI_DCS_SOFT_RESET);
	mdelay(5);
	write_reg(par, MIPI_DCS_SET_DISPLAY_OFF);
	mdelay(100);

	write_reg(par, 0xFE);
	write_reg(par, 0xEF); 

	write_reg(par, 0x80, 0xFF);
	write_reg(par, 0x81, 0xFF);
	write_reg(par, 0x82, 0xFF);
	write_reg(par, 0x83, 0xFF);
	write_reg(par, 0x84, 0xFF); 
	write_reg(par, 0x85, 0xFF); 
	write_reg(par, 0x86, 0xFF); 
	write_reg(par, 0x87, 0xFF);
	write_reg(par, 0x88, 0xFF);
	write_reg(par, 0x89, 0xFF);
	write_reg(par, 0x8A, 0xFF);
	write_reg(par, 0x8B, 0xFF);
	write_reg(par, 0x8C, 0xFF);
	write_reg(par, 0x8D, 0xFF);
	write_reg(par, 0x8E, 0xFF); 
	write_reg(par, 0x8F, 0xFF); 

	write_reg(par, 0x3A, 0x05);
	write_reg(par, 0xEC, 0x01);

	write_reg(par, 0x74, 0x02, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00);
	write_reg(par, 0x98, 0x3E);
	write_reg(par, 0x99, 0x3E);

	write_reg(par, 0xB5, 0x0D, 0x0D);

	write_reg(par, 0x60, 0x38, 0x0F, 0x79, 0x67);
	write_reg(par, 0x61, 0x38, 0x11, 0x79, 0x67);
	write_reg(par, 0x64, 0x38, 0x17, 0x71, 0x5F, 0x79, 0x67);
	write_reg(par, 0x65, 0x38, 0x13, 0x71, 0x5B, 0x79, 0x67);
	write_reg(par, 0x6A, 0x00, 0x00);
	write_reg(par, 0x6C, 0x22, 0x02, 0x22, 0x02, 0x22, 0x22, 0x50);
	write_reg(par, 0x6E, 0x03, 0x03, 0x01, 0x01, 0x00, 0x00, 0x0f, 0x0f, 0x0d, 0x0d, 0x0b , 0x0b, 0x09, 0x09, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x0a, 0x0c, 0x0c, 0x0e, 0x0e, 0x10, 0x10, 0x00, 0x00, 0x02, 0x02, 0x04, 0x04);

	write_reg(par, 0xbf, 0x01);
	write_reg(par, 0xF9, 0x40);
	write_reg(par, 0x9b, 0x3b);
	write_reg(par, 0x93, 0x33, 0x7f, 0x00);

	write_reg(par, 0x7E, 0x30);
	write_reg(par, 0x70, 0x0d, 0x02, 0x08, 0x0d, 0x02, 0x08);
	write_reg(par, 0x71, 0x0d, 0x02, 0x08);
	write_reg(par, 0x91, 0x0E, 0x09);
	write_reg(par, 0xc3, 0x19);
	write_reg(par, 0xc4, 0x19);
	write_reg(par, 0xc9, 0x3c);
	write_reg(par, 0xf0, 0x53, 0x15, 0x0a, 0x04, 0x00, 0x3e);
	write_reg(par, 0xf2, 0x53, 0x15, 0x0a, 0x04, 0x00, 0x3a);
	write_reg(par, 0xf1, 0x56, 0xa8, 0x7f, 0x33, 0x34, 0x5f);
	write_reg(par, 0xf3, 0x52, 0xa4, 0x7f, 0x33, 0x34, 0xdf);

	write_reg(par, 0x36, 0x00);
	write_reg(par, 0x11);
	mdelay(200); 
	write_reg(par, 0x29);
	write_reg(par, 0x2C);

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	write_reg(par, MIPI_DCS_SET_COLUMN_ADDRESS,
		  (xs >> 8) & 0xFF, xs & 0xFF, (xe >> 8) & 0xFF, xe & 0xFF);

	write_reg(par, MIPI_DCS_SET_PAGE_ADDRESS,
		  (ys >> 8) & 0xFF, ys & 0xFF, (ye >> 8) & 0xFF, ye & 0xFF);

	write_reg(par, MIPI_DCS_WRITE_MEMORY_START);
}

#define MEM_Y   BIT(7) /* MY row address order */
#define MEM_X   BIT(6) /* MX column address order */
#define MEM_V   BIT(5) /* MV row / column exchange */
#define MEM_L   BIT(4) /* ML vertical refresh order */
#define MEM_H   BIT(2) /* MH horizontal refresh order */
#define MEM_BGR (3) /* RGB-BGR Order */
static int set_var(struct fbtft_par *par)
{
	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE, (par->bgr << MEM_BGR));
		break;
	case 270:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_V | MEM_L | (par->bgr << MEM_BGR));
		break;
	case 180:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_Y | (par->bgr << MEM_BGR));
		break;
	case 90:
		write_reg(par, MIPI_DCS_SET_ADDRESS_MODE,
			  MEM_Y | MEM_V | (par->bgr << MEM_BGR));
		break;
	}

	return 0;
}

/*
 * Gamma string format:
 *  Positive: Par1 Par2 [...] Par15
 *  Negative: Par1 Par2 [...] Par15
 */
#define CURVE(num, idx)  curves[(num) * par->gamma.num_values + (idx)]
static int set_gamma(struct fbtft_par *par, u32 *curves)
{
/*
	int i;

	for (i = 0; i < par->gamma.num_curves; i++)
		write_reg(par, 0xE0 + i,
			  CURVE(i, 0), CURVE(i, 1), CURVE(i, 2),
			  CURVE(i, 3), CURVE(i, 4), CURVE(i, 5),
			  CURVE(i, 6), CURVE(i, 7), CURVE(i, 8),
			  CURVE(i, 9), CURVE(i, 10), CURVE(i, 11),
			  CURVE(i, 12), CURVE(i, 13), CURVE(i, 14));
*/
	return 0;
}

#undef CURVE

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.txbuflen = TXBUFLEN,
	.gamma_num = 2,
	.gamma_len = 15,
	.gamma = DEFAULT_GAMMA,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
		.set_gamma = set_gamma,
	},
};

FBTFT_REGISTER_DRIVER(DRVNAME, "galaxycore,gc9d01", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:gc9d01");
MODULE_ALIAS("platform:gc9d01");

MODULE_DESCRIPTION("FB driver for the gc9d01 LCD display controller");
MODULE_AUTHOR("agepan_ft_miku");
MODULE_LICENSE("GPL");

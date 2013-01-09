/***************************************************************************
 *   Copyright (C) 2013 Franck Jullien                                     *
 *   elec4fun@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef OR1K_DU
#define OR1K_DU

#include "helper/types.h"

struct or1k_du {
	const char *name;
	struct list_head list;
	struct jtag_tap *tap;

	int (*or1k_jtag_init)(struct or1k_jtag *jtag_info);

	int (*or1k_jtag_read_cpu)(struct or1k_jtag *jtag_info,
				  uint32_t addr, int count, uint32_t *value);

	int (*or1k_jtag_write_cpu)(struct or1k_jtag *jtag_info,
				   uint32_t addr, int count, const uint32_t *value);

	int (*or1k_jtag_read_cpu_cr)(struct or1k_jtag *jtag_info, uint32_t *value);

	int (*or1k_jtag_write_cpu_cr)(struct or1k_jtag *jtag_info, uint32_t stall,
				      uint32_t reset);

	int (*or1k_jtag_read_memory32)(struct or1k_jtag *jtag_info, uint32_t addr,
					int count, uint32_t *buffer);

	int (*or1k_jtag_read_memory16)(struct or1k_jtag *jtag_info, uint32_t addr,
					int count, uint16_t *buffer);

	int (*or1k_jtag_read_memory8)(struct or1k_jtag *jtag_info, uint32_t addr,
				       int count, uint8_t *buffer);

	int (*or1k_jtag_write_memory32)(struct or1k_jtag *jtag_info, uint32_t addr,
					 int count, const uint32_t *buffer);

	int (*or1k_jtag_write_memory16)(struct or1k_jtag *jtag_info, uint32_t addr,
					 int count, const uint16_t *buffer);

	int (*or1k_jtag_write_memory8)(struct or1k_jtag *jtag_info, uint32_t addr,
					int count, const uint8_t *buffer);
};

static inline struct or1k_du *or1k_jtag_to_du(struct or1k_jtag *jtag_info)
{
	return (struct or1k_du *)jtag_info->du_core;
}

static inline struct or1k_du *or1k_to_du(struct or1k_common *or1k)
{
	struct or1k_jtag *jtag = &or1k->jtag;
	return (struct or1k_du *)jtag->du_core;
}

#endif /* OR1K_DU */


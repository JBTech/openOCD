/***************************************************************************
 *   Copyright (C) 2011 by Julius Baxter                                   *
 *   julius@opencores.org                                                  *
 *                                                                         *
 *   Copyright (C) 2012 by Franck Jullien                                  *
 *   elec4fun@gmail.com                                                    *
 *                                                                         *
 *   Copyright (C) 2012 by Marek Czerski                                   *
 *   ma.czerski@gmail.com                                                  *
 *                                                                         *
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "jtag/jtag.h"
#include "register.h"
#include "algorithm.h"
#include "target.h"
#include "tdesc.h"
#include "breakpoints.h"
#include "target_type.h"
#include "or1k_jtag.h"
#include "or1k.h"

#include <helper/time_support.h>
#include "server/server.h"
#include "server/gdb_server.h"

#include "fileio.h"

static int or1k_read_core_reg(struct target *target, int num);
static int or1k_write_core_reg(struct target *target, int num);

struct or1k_core_reg *or1k_core_reg_list_arch_info;

struct or1k_core_reg_init or1k_init_reg_list[] = {
	{"r0"       , GROUP0 + 1024, "group0", NULL},
	{"r1"       , GROUP0 + 1025, "group0", NULL},
	{"r2"       , GROUP0 + 1026, "group0", NULL},
	{"r3"       , GROUP0 + 1027, "group0", NULL},
	{"r4"       , GROUP0 + 1028, "group0", NULL},
	{"r5"       , GROUP0 + 1029, "group0", NULL},
	{"r6"       , GROUP0 + 1030, "group0", NULL},
	{"r7"       , GROUP0 + 1031, "group0", NULL},
	{"r8"       , GROUP0 + 1032, "group0", NULL},
	{"r9"       , GROUP0 + 1033, "group0", NULL},
	{"r10"      , GROUP0 + 1034, "group0", NULL},
	{"r11"      , GROUP0 + 1035, "group0", NULL},
	{"r12"      , GROUP0 + 1036, "group0", NULL},
	{"r13"      , GROUP0 + 1037, "group0", NULL},
	{"r14"      , GROUP0 + 1038, "group0", NULL},
	{"r15"      , GROUP0 + 1039, "group0", NULL},
	{"r16"      , GROUP0 + 1040, "group0", NULL},
	{"r17"      , GROUP0 + 1041, "group0", NULL},
	{"r18"      , GROUP0 + 1042, "group0", NULL},
	{"r19"      , GROUP0 + 1043, "group0", NULL},
	{"r20"      , GROUP0 + 1044, "group0", NULL},
	{"r21"      , GROUP0 + 1045, "group0", NULL},
	{"r22"      , GROUP0 + 1046, "group0", NULL},
	{"r23"      , GROUP0 + 1047, "group0", NULL},
	{"r24"      , GROUP0 + 1048, "group0", NULL},
	{"r25"      , GROUP0 + 1049, "group0", NULL},
	{"r26"      , GROUP0 + 1050, "group0", NULL},
	{"r27"      , GROUP0 + 1051, "group0", NULL},
	{"r28"      , GROUP0 + 1052, "group0", NULL},
	{"r29"      , GROUP0 + 1053, "group0", NULL},
	{"r30"      , GROUP0 + 1054, "group0", NULL},
	{"r31"      , GROUP0 + 1055, "group0", NULL},
	{"ppc"      , GROUP0 + 18,   "group0", NULL},
	{"npc"      , GROUP0 + 16,   "group0", NULL},
	{"sr"       , GROUP0 + 17,   "group0", NULL},
	{"vr"       , GROUP0 + 0,    "group0", "system"},
	{"upr"      , GROUP0 + 1,    "group0", "system"},
	{"cpucfgr"  , GROUP0 + 2,    "group0", "system"},
	{"dmmucfgr" , GROUP0 + 3,    "group0", "system"},
	{"immucfgr" , GROUP0 + 4,    "group0", "system"},
	{"dccfgr"   , GROUP0 + 5,    "group0", "system"},
	{"iccfgr"   , GROUP0 + 6,    "group0", "system"},
	{"dcfgr"    , GROUP0 + 7,    "group0", "system"},
	{"pccfgr"   , GROUP0 + 8,    "group0", "system"},
	{"fpcsr"    , GROUP0 + 20,   "group0", "system"},
	{"epcr0"    , GROUP0 + 32,   "group0", "system"},
	{"epcr1"    , GROUP0 + 33,   "group0", "system"},
	{"epcr2"    , GROUP0 + 34,   "group0", "system"},
	{"epcr3"    , GROUP0 + 35,   "group0", "system"},
	{"epcr4"    , GROUP0 + 36,   "group0", "system"},
	{"epcr5"    , GROUP0 + 37,   "group0", "system"},
	{"epcr6"    , GROUP0 + 38,   "group0", "system"},
	{"epcr7"    , GROUP0 + 39,   "group0", "system"},
	{"epcr8"    , GROUP0 + 40,   "group0", "system"},
	{"epcr9"    , GROUP0 + 41,   "group0", "system"},
	{"epcr10"   , GROUP0 + 42,   "group0", "system"},
	{"epcr11"   , GROUP0 + 43,   "group0", "system"},
	{"epcr12"   , GROUP0 + 44,   "group0", "system"},
	{"epcr13"   , GROUP0 + 45,   "group0", "system"},
	{"epcr14"   , GROUP0 + 46,   "group0", "system"},
	{"epcr15"   , GROUP0 + 47,   "group0", "system"},
	{"eear0"    , GROUP0 + 48,   "group0", "system"},
	{"eear1"    , GROUP0 + 49,   "group0", "system"},
	{"eear2"    , GROUP0 + 50,   "group0", "system"},
	{"eear3"    , GROUP0 + 51,   "group0", "system"},
	{"eear4"    , GROUP0 + 52,   "group0", "system"},
	{"eear5"    , GROUP0 + 53,   "group0", "system"},
	{"eear6"    , GROUP0 + 54,   "group0", "system"},
	{"eear7"    , GROUP0 + 55,   "group0", "system"},
	{"eear8"    , GROUP0 + 56,   "group0", "system"},
	{"eear9"    , GROUP0 + 57,   "group0", "system"},
	{"eear10"   , GROUP0 + 58,   "group0", "system"},
	{"eear11"   , GROUP0 + 59,   "group0", "system"},
	{"eear12"   , GROUP0 + 60,   "group0", "system"},
	{"eear13"   , GROUP0 + 61,   "group0", "system"},
	{"eear14"   , GROUP0 + 62,   "group0", "system"},
	{"eear15"   , GROUP0 + 63,   "group0", "system"},
	{"esr0"     , GROUP0 + 64,   "group0", "system"},
	{"esr1"     , GROUP0 + 65,   "group0", "system"},
	{"esr2"     , GROUP0 + 66,   "group0", "system"},
	{"esr3"     , GROUP0 + 67,   "group0", "system"},
	{"esr4"     , GROUP0 + 68,   "group0", "system"},
	{"esr5"     , GROUP0 + 69,   "group0", "system"},
	{"esr6"     , GROUP0 + 70,   "group0", "system"},
	{"esr7"     , GROUP0 + 71,   "group0", "system"},
	{"esr8"     , GROUP0 + 72,   "group0", "system"},
	{"esr9"     , GROUP0 + 73,   "group0", "system"},
	{"esr10"    , GROUP0 + 74,   "group0", "system"},
	{"esr11"    , GROUP0 + 75,   "group0", "system"},
	{"esr12"    , GROUP0 + 76,   "group0", "system"},
	{"esr13"    , GROUP0 + 77,   "group0", "system"},
	{"esr14"    , GROUP0 + 78,   "group0", "system"},
	{"esr15"    , GROUP0 + 79,   "group0", "system"},

	{"dmmuucr"  , GROUP1 + 0,    "group1", "dmmu"},
	{"dmmuupr"  , GROUP1 + 1,    "group1", "dmmu"},
	{"dtlbeir"  , GROUP1 + 2,    "group1", "dmmu"},
	{"datbmr0"  , GROUP1 + 4,    "group1", "dmmu"},
	{"datbmr1"  , GROUP1 + 5,    "group1", "dmmu"},
	{"datbmr2"  , GROUP1 + 6,    "group1", "dmmu"},
	{"datbmr3"  , GROUP1 + 7,    "group1", "dmmu"},
	{"datbtr0"  , GROUP1 + 8,    "group1", "dmmu"},
	{"datbtr1"  , GROUP1 + 9,    "group1", "dmmu"},
	{"datbtr2"  , GROUP1 + 10,   "group1", "dmmu"},
	{"datbtr3"  , GROUP1 + 11,   "group1", "dmmu"},

	{"immucr"   , GROUP2 + 0,    "group2", "immu"},
	{"immupr"   , GROUP2 + 1,    "group2", "immu"},
	{"itlbeir"  , GROUP2 + 2,    "group2", "immu"},
	{"iatbmr0"  , GROUP2 + 4,    "group2", "immu"},
	{"iatbmr1"  , GROUP2 + 5,    "group2", "immu"},
	{"iatbmr2"  , GROUP2 + 6,    "group2", "immu"},
	{"iatbmr3"  , GROUP2 + 7,    "group2", "immu"},
	{"iatbtr0"  , GROUP2 + 8,    "group2", "immu"},
	{"iatbtr1"  , GROUP2 + 9,    "group2", "immu"},
	{"iatbtr2"  , GROUP2 + 10,   "group2", "immu"},
	{"iatbtr3"  , GROUP2 + 11,   "group2", "immu"},

	{"dccr"     , GROUP3 + 0,    "group3", "dcache"},
	{"dcbpr"    , GROUP3 + 1,    "group3", "dcache"},
	{"dcbfr"    , GROUP3 + 2,    "group3", "dcache"},
	{"dcbir"    , GROUP3 + 3,    "group3", "dcache"},
	{"dcbwr"    , GROUP3 + 4,    "group3", "dcache"},
	{"dcblr"    , GROUP3 + 5,    "group3", "dcache"},

	{"iccr"     , GROUP4 + 0,    "group4", "icache"},
	{"icbpr"    , GROUP4 + 1,    "group4", "icache"},
	{"icbir"    , GROUP4 + 2,    "group4", "icache"},
	{"icblr"    , GROUP4 + 3,    "group4", "icache"},

	{"maclo"    , GROUP5 + 0,    "group5", "mac"},
	{"machi"    , GROUP5 + 1,    "group5", "mac"},

	{"dvr0"     , GROUP6 + 0,    "group6", "debug"},
	{"dvr1"     , GROUP6 + 1,    "group6", "debug"},
	{"dvr2"     , GROUP6 + 2,    "group6", "debug"},
	{"dvr3"     , GROUP6 + 3,    "group6", "debug"},
	{"dvr4"     , GROUP6 + 4,    "group6", "debug"},
	{"dvr5"     , GROUP6 + 5,    "group6", "debug"},
	{"dvr6"     , GROUP6 + 6,    "group6", "debug"},
	{"dvr7"     , GROUP6 + 7,    "group6", "debug"},
	{"dcr0"     , GROUP6 + 8,    "group6", "debug"},
	{"dcr1"     , GROUP6 + 9,    "group6", "debug"},
	{"dcr2"     , GROUP6 + 10,   "group6", "debug"},
	{"dcr3"     , GROUP6 + 11,   "group6", "debug"},
	{"dcr4"     , GROUP6 + 12,   "group6", "debug"},
	{"dcr5"     , GROUP6 + 13,   "group6", "debug"},
	{"dcr6"     , GROUP6 + 14,   "group6", "debug"},
	{"dcr7"     , GROUP6 + 15,   "group6", "debug"},
	{"dmr1"     , GROUP6 + 16,   "group6", "debug"},
	{"dmr2"     , GROUP6 + 17,   "group6", "debug"},
	{"dcwr0"    , GROUP6 + 18,   "group6", "debug"},
	{"dcwr1"    , GROUP6 + 19,   "group6", "debug"},
	{"dsr"      , GROUP6 + 20,   "group6", "debug"},
	{"drr"      , GROUP6 + 21,   "group6", "debug"},

	{"pccr0"    , GROUP7 + 0,    "group7", "perf"},
	{"pccr1"    , GROUP7 + 1,    "group7", "perf"},
	{"pccr2"    , GROUP7 + 2,    "group7", "perf"},
	{"pccr3"    , GROUP7 + 3,    "group7", "perf"},
	{"pccr4"    , GROUP7 + 4,    "group7", "perf"},
	{"pccr5"    , GROUP7 + 5,    "group7", "perf"},
	{"pccr6"    , GROUP7 + 6,    "group7", "perf"},
	{"pccr7"    , GROUP7 + 7,    "group7", "perf"},
	{"pcmr0"    , GROUP7 + 8,    "group7", "perf"},
	{"pcmr1"    , GROUP7 + 9,    "group7", "perf"},
	{"pcmr2"    , GROUP7 + 10,   "group7", "perf"},
	{"pcmr3"    , GROUP7 + 11,   "group7", "perf"},
	{"pcmr4"    , GROUP7 + 12,   "group7", "perf"},
	{"pcmr5"    , GROUP7 + 13,   "group7", "perf"},
	{"pcmr6"    , GROUP7 + 14,   "group7", "perf"},
	{"pcmr7"    , GROUP7 + 15,   "group7", "perf"},

	{"pmr"      , GROUP8 + 0,    "group8", "power"},

	{"picmr"    , GROUP9 + 0,    "group9", "pic"},
	{"picsr"    , GROUP9 + 2,    "group9", "pic"},

	{"ttmr"     , GROUP10 + 0,   "group10", "timer"},
	{"ttcr"     , GROUP10 + 1,   "group10", "timer"},
};

static int or1k_add_reg(struct target *target, struct or1k_core_reg *new_reg)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int reg_list_size = or1k->nb_regs * sizeof(struct or1k_core_reg);

	or1k_core_reg_list_arch_info = realloc(or1k_core_reg_list_arch_info,
				reg_list_size + sizeof(struct or1k_core_reg));

	memcpy(&or1k_core_reg_list_arch_info[or1k->nb_regs], new_reg,
		sizeof(struct or1k_core_reg));

	or1k_core_reg_list_arch_info[or1k->nb_regs].list_num = or1k->nb_regs;

	or1k->nb_regs++;

	return ERROR_OK;
}

static int or1k_create_reg_list(struct target *target)
{
	struct or1k_common *or1k = target_to_or1k(target);
	struct or1k_core_reg new_reg;
	int way;
	char name[32];
	uint32_t i;

	or1k_core_reg_list_arch_info = malloc(ARRAY_SIZE(or1k_init_reg_list) *
				       sizeof(struct or1k_core_reg));

	if (or1k_core_reg_list_arch_info == NULL)
		return ERROR_FAIL;

	for (i = 0; i < ARRAY_SIZE(or1k_init_reg_list); i++) {
		or1k_core_reg_list_arch_info[i].name = or1k_init_reg_list[i].name;
		or1k_core_reg_list_arch_info[i].spr_num = or1k_init_reg_list[i].spr_num;
		or1k_core_reg_list_arch_info[i].group = or1k_init_reg_list[i].group;
		or1k_core_reg_list_arch_info[i].feature = or1k_init_reg_list[i].feature;
		or1k_core_reg_list_arch_info[i].list_num = i;
		or1k_core_reg_list_arch_info[i].target = NULL;
		or1k_core_reg_list_arch_info[i].or1k_common = NULL;
	}

	or1k->nb_regs = ARRAY_SIZE(or1k_init_reg_list);

	new_reg.target = NULL;
	new_reg.or1k_common = NULL;

	for (way = 0; way < 4; way++) {
		for (i = 0; i < 128; i++) {

			sprintf(name, "dtlbw%dmr%d", way, i);
			new_reg.name = strdup(name);
			new_reg.spr_num = GROUP1 + 512 + i + (way * 256);
			new_reg.feature = "group1";
			new_reg.group = "dmmu";
			or1k_add_reg(target, &new_reg);

			sprintf(name, "dtlbw%dtr%d", way, i);
			new_reg.name = strdup(name);
			new_reg.spr_num = GROUP1 + 640 + i + (way * 256);
			new_reg.feature = "group1";
			new_reg.group = "dmmu";
			or1k_add_reg(target, &new_reg);


			sprintf(name, "itlbw%dmr%d", way, i);
			new_reg.name = strdup(name);
			new_reg.spr_num = GROUP2 + 512 + i + (way * 256);
			new_reg.feature = "group2";
			new_reg.group = "immu";
			or1k_add_reg(target, &new_reg);


			sprintf(name, "itlbw%dtr%d", way, i);
			new_reg.name = strdup(name);
			new_reg.spr_num = GROUP2 + 640 + i + (way * 256);
			new_reg.feature = "group2";
			new_reg.group = "immu";
			or1k_add_reg(target, &new_reg);

		}
	}

	return ERROR_OK;
}

static int or1k_jtag_read_regs(struct or1k_jtag *jtag_info, uint32_t *regs)
{
	int retval;

	retval = or1k_jtag_read_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_R0].spr_num, OR1K_REG_R31+1,
			regs+OR1K_REG_R0);
	if (retval != ERROR_OK)
		return retval;

	return ERROR_OK;
}

static int or1k_jtag_write_regs(struct or1k_jtag *jtag_info, uint32_t *regs)
{
	int retval;

	retval = or1k_jtag_write_cpu(jtag_info,
			or1k_core_reg_list_arch_info[OR1K_REG_R0].spr_num, OR1K_REG_R31+1,
			&regs[OR1K_REG_R0]);
	if (retval != ERROR_OK)
		return retval;

	return ERROR_OK;
}

int or1k_save_context(struct target *target)
{
	int retval, i, regs_read = 0;
	struct or1k_common *or1k = target_to_or1k(target);

	for (i = 0; i < OR1KNUMCOREREGS; i++) {
		if (!or1k->core_cache->reg_list[i].valid) {
			if (i == OR1K_REG_PPC || i == OR1K_REG_NPC || i == OR1K_REG_SR) {
				or1k_jtag_read_cpu(&or1k->jtag,
						or1k_core_reg_list_arch_info[i].spr_num, 1,
						&or1k->core_regs[i]);
			} else if (!regs_read) {
				/* read gpr registers at once (but only one time in this loop) */
				retval = or1k_jtag_read_regs(&or1k->jtag, or1k->core_regs);
				if (retval != ERROR_OK)
					return retval;
				/* prevent next reads in this loop */
				regs_read = 1;
			}
			/* We've just updated the core_reg[i], now update
			   the core cache */
			or1k_read_core_reg(target, i);
		}
	}

	return ERROR_OK;
}


int or1k_restore_context(struct target *target)
{
	int retval, i, reg_write = 0;
	struct or1k_common *or1k = target_to_or1k(target);

	for (i = 0; i < OR1KNUMCOREREGS; i++) {
		if (or1k->core_cache->reg_list[i].dirty) {
			or1k_write_core_reg(target, i);

			if (i == OR1K_REG_PPC || i == OR1K_REG_NPC || i == OR1K_REG_SR) {
				or1k_jtag_write_cpu(&or1k->jtag,
						or1k_core_reg_list_arch_info[i].spr_num, 1,
						&or1k->core_regs[i]);
			} else
				reg_write = 1;
		}
	}

	if (reg_write) {
		/* read gpr registers at once (but only one time in this loop) */
		retval = or1k_jtag_write_regs(&or1k->jtag, or1k->core_regs);
		if (retval != ERROR_OK)
			return retval;
	}

	return ERROR_OK;
}

static int or1k_read_core_reg(struct target *target, int num)
{
	uint32_t reg_value;
	int retval;
	struct or1k_common *or1k = target_to_or1k(target);

	if ((num < 0) || (num >= or1k->nb_regs))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if ((num >= 0) && (num < OR1KNUMCOREREGS)) {
		reg_value = or1k->core_regs[num];
		buf_set_u32(or1k->core_cache->reg_list[num].value, 0, 32, reg_value);
		LOG_DEBUG("read core reg %i value 0x%" PRIx32 "", num , reg_value);
		or1k->core_cache->reg_list[num].valid = 1;
		or1k->core_cache->reg_list[num].dirty = 0;
	} else {
		/* This is an spr, always read value from HW */
		retval = or1k_jtag_read_cpu(&or1k->jtag, or1k_core_reg_list_arch_info[num].spr_num, 1, &reg_value);
		if (retval != ERROR_OK)
			return retval;
		buf_set_u32(or1k->core_cache->reg_list[num].value, 0, 32, reg_value);
		LOG_DEBUG("read spr reg %i value 0x%" PRIx32 "", num , reg_value);
	}

	return ERROR_OK;
}

static int or1k_write_core_reg(struct target *target, int num)
{
	uint32_t reg_value;
	struct or1k_common *or1k = target_to_or1k(target);

	if ((num < 0) || (num >= OR1KNUMCOREREGS))
		return ERROR_COMMAND_SYNTAX_ERROR;

	reg_value = buf_get_u32(or1k->core_cache->reg_list[num].value, 0, 32);
	or1k->core_regs[num] = reg_value;
	LOG_DEBUG("write core reg %i value 0x%" PRIx32 "", num , reg_value);
	or1k->core_cache->reg_list[num].valid = 1;
	or1k->core_cache->reg_list[num].dirty = 0;

	return ERROR_OK;
}

static int or1k_get_core_reg(struct reg *reg)
{
	struct or1k_core_reg *or1k_reg = reg->arch_info;
	struct target *target = or1k_reg->target;

	if (target->state != TARGET_HALTED)
		return ERROR_TARGET_NOT_HALTED;

	return or1k_read_core_reg(target, or1k_reg->list_num);
}

static int or1k_set_core_reg(struct reg *reg, uint8_t *buf)
{
	struct or1k_core_reg *or1k_reg = reg->arch_info;
	struct target *target = or1k_reg->target;
	struct or1k_common *or1k = target_to_or1k(target);
	uint32_t value = buf_get_u32(buf, 0, 32);
	int retval;

	if (target->state != TARGET_HALTED)
		return ERROR_TARGET_NOT_HALTED;

	if (or1k_reg->list_num < OR1KNUMCOREREGS) {
		buf_set_u32(reg->value, 0, 32, value);
		reg->dirty = 1;
		reg->valid = 1;
	} else {
		/* This is an spr, write it to the HW */
		retval = or1k_jtag_write_cpu(&or1k->jtag, or1k_reg->spr_num, 1, &value);
		if (retval != ERROR_OK)
			return retval;
	}

	return ERROR_OK;
}

static const struct reg_arch_type or1k_reg_type = {
	.get = or1k_get_core_reg,
	.set = or1k_set_core_reg,
};

static struct reg_cache *or1k_build_reg_cache(struct target *target)
{
	int num_regs = OR1KNUMCOREREGS;
	struct or1k_common *or1k = target_to_or1k(target);
	struct reg_cache **cache_p = register_get_last_cache_p(&target->reg_cache);
	struct reg_cache *cache = malloc(sizeof(struct reg_cache));
	struct reg *reg_list = malloc((or1k->nb_regs) * sizeof(struct reg));
	struct or1k_core_reg *arch_info =
		malloc((or1k->nb_regs) * sizeof(struct or1k_core_reg));
	int i;

	/* Build the process context cache */
	cache->name = "openrisc 1000 registers";
	cache->next = NULL;
	cache->reg_list = reg_list;
	cache->num_regs = num_regs;
	(*cache_p) = cache;
	or1k->core_cache = cache;

	for (i = 0; i < or1k->nb_regs; i++) {
		arch_info[i] = or1k_core_reg_list_arch_info[i];
		arch_info[i].target = target;
		arch_info[i].or1k_common = or1k;
		reg_list[i].name = or1k_core_reg_list_arch_info[i].name;
		reg_list[i].feature = or1k_core_reg_list_arch_info[i].feature;
		reg_list[i].group = or1k_core_reg_list_arch_info[i].group;
		reg_list[i].size = 32;
		reg_list[i].value = calloc(1, 4);
		reg_list[i].dirty = 0;
		reg_list[i].valid = 0;
		reg_list[i].type = &or1k_reg_type;
		reg_list[i].arch_info = &arch_info[i];
	}

	return cache;
}

static int or1k_generate_tdesc(struct target *target, const char *filename)
{
	struct fileio fileio;
	char **features = NULL;
	int current_feature = 0;
	int retval;

	/* Create the tdesc file, prepare the header and set the architecture
	 * to or32 in the file.
	 */
	retval = open_and_init_tdesc_file(&fileio, filename, "or32");
	if (retval != ERROR_OK)
		return retval;

	/* Get a list of available target registers features */
	retval = get_reg_features_list(target, &features);
	if (retval < 0)
		return ERROR_FAIL;

	/* If we found some features associated with registers, create sections */
	if (features != NULL) {
		while (features[current_feature]) {
			retval = generate_feature_section(target, &fileio, "or32", features[current_feature]);
			if (retval != ERROR_OK)
				return retval;
			current_feature++;
		}
	}

	/* For registers without features, put them in "nogroup" feature */
	if (count_reg_without_group(target) > 0) {
		retval = generate_feature_section(target, &fileio, "or32", NULL);
		if (retval != ERROR_OK)
			return retval;
	}

	free(features);
	close_tdesc_file(&fileio);

	return ERROR_OK;
}

static int or1k_debug_entry(struct target *target)
{
	/* Perhaps do more debugging entry (processor stalled) set up here */
	or1k_save_context(target);
	return ERROR_OK;
}

static int or1k_halt(struct target *target)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int retval;

	LOG_DEBUG("target->state: %s",
		  target_state_name(target));

	if (target->state == TARGET_HALTED) {
		LOG_DEBUG("target was already halted");
		return ERROR_OK;
	}

	if (target->state == TARGET_UNKNOWN)
		LOG_WARNING("target was in unknown state when halt was requested");

	if (target->state == TARGET_RESET) {
		if ((jtag_get_reset_config() & RESET_SRST_PULLS_TRST) &&
		    jtag_get_srst()) {
			LOG_ERROR("can't request a halt while in reset if nSRST pulls nTRST");
			return ERROR_TARGET_FAILURE;
		} else {
			target->debug_reason = DBG_REASON_DBGRQ;
			return ERROR_OK;
		}
	}

	/* Mohor debug unit-specific. */
	retval = or1k_jtag_write_cpu_cr(&or1k->jtag, 1, 0);
	if (retval != ERROR_OK)
		return retval;

	target->debug_reason = DBG_REASON_DBGRQ;

	return ERROR_OK;
}

static int or1k_is_cpu_running(struct target *target, int* running)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int retval;
	uint32_t cpu_cr;

	/* Have a retry loop to determine of the CPU is running.
	   If target has been hard reset for any reason, it might take a couple
	   of goes before it's ready again.
	*/

	int tries = 0;
	const int RETRIES_MAX = 5;
	while (tries < RETRIES_MAX) {

		tries++;
		/* Specific to Mohor debug interface - others may have to do
		 * something different here.
		 */
		retval = or1k_jtag_read_cpu_cr(&or1k->jtag, &cpu_cr);

		if (retval != ERROR_OK) {
			LOG_WARNING("Debug IF CPU control reg read failure.");
			/* Try once to restart the JTAG infrastructure -
			   quite possibly the board has just been reset. */
			LOG_WARNING("Resetting JTAG TAP state and reconnectiong to debug IF.");
			or1k_jtag_init(&or1k->jtag);

			LOG_WARNING("attempt %d of %d", tries, RETRIES_MAX);

			/* TODO: perhaps some sort of time delay here. 1s? */
			sleep(1);

			continue;

		} else {
			/* Specific to Mohor debug interface */
			*running = !(cpu_cr & OR1K_MOHORDBGIF_CPU_CR_STALL);
			return retval;
		}
	}

	LOG_WARNING("Could not re-establish communication with target");
	return retval;
}

static int or1k_poll(struct target *target)
{

	int retval;
	int running;

	retval = or1k_is_cpu_running(target, &running);
	if (retval != ERROR_OK)
		return retval;

	/* check for processor halted */
	if (!running) {
		/* It's actually stalled, so update our software's state */
		if ((target->state == TARGET_RUNNING) ||
		    (target->state == TARGET_RESET)) {

			target->state = TARGET_HALTED;

			retval = or1k_debug_entry(target);
			if (retval != ERROR_OK)
				return retval;

			target_call_event_callbacks(target,
						    TARGET_EVENT_HALTED);
		} else if (target->state == TARGET_DEBUG_RUNNING) {
			target->state = TARGET_HALTED;

			retval = or1k_debug_entry(target);
			if (retval != ERROR_OK)
				return retval;

			target_call_event_callbacks(target,
						    TARGET_EVENT_DEBUG_HALTED);
		}
	} else { /* ... target is running */

		/* If target was supposed to be stalled, stall it again */
		if  (target->state == TARGET_HALTED) {

			target->state = TARGET_RUNNING;

			retval = or1k_halt(target);
			if (retval != ERROR_OK)
				return retval;

			retval = or1k_debug_entry(target);
			if (retval != ERROR_OK)
				return retval;

			target_call_event_callbacks(target,
						    TARGET_EVENT_DEBUG_HALTED);
		}

		target->state = TARGET_RUNNING;

	}

	return ERROR_OK;
}

static int or1k_assert_reset(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);
	return ERROR_OK;
}

static int or1k_deassert_reset(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);
	return ERROR_OK;
}

static int or1k_soft_reset_halt(struct target *target)
{
	LOG_ERROR("%s: implement me", __func__);
	return ERROR_OK;
}

static int or1k_resume_or_step(struct target *target, int current,
			       uint32_t address, int handle_breakpoints,
			       int debug_execution, int step)
{
	struct or1k_common *or1k = target_to_or1k(target);
	struct breakpoint *breakpoint = NULL;
	uint32_t resume_pc;
	int retval;
	uint32_t debug_reg_list[OR1K_DEBUG_REG_NUM];

	LOG_DEBUG(" addr: 0x%x, stepping: %d, handle breakpoints %d\n",
		  address, step, handle_breakpoints);

	if (target->state != TARGET_HALTED) {
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	if (!debug_execution)
		target_free_all_working_areas(target);

	/* current ? continue on current pc : continue at <address> */
	if (!current)
		buf_set_u32(or1k->core_cache->reg_list[OR1K_REG_NPC].value, 0,
			    32, address);

	if (!step)
		or1k_restore_context(target);

	/* read debug registers (starting from DMR1 register) */
	or1k_jtag_read_cpu(&or1k->jtag, OR1K_DMR1_CPU_REG_ADD, OR1K_DEBUG_REG_NUM, debug_reg_list);

	/* Clear Debug Reason Register (DRR) */
	debug_reg_list[OR1K_DEBUG_REG_DRR] = 0;

	/* Clear watchpoint break generation in Debug Mode Register 2 (DMR2) */
	debug_reg_list[OR1K_DEBUG_REG_DMR2] &= ~OR1K_DMR2_WGB;
	if (step)
		/* Set the single step trigger in Debug Mode Register 1 (DMR1) */
		debug_reg_list[OR1K_DEBUG_REG_DMR1] |= OR1K_DMR1_ST | OR1K_DMR1_BT;
	else
		/* Clear the single step trigger in Debug Mode Register 1 (DMR1) */
		debug_reg_list[OR1K_DEBUG_REG_DMR1] &= ~(OR1K_DMR1_ST | OR1K_DMR1_BT);

	/* Set traps to be handled by the debug unit in the Debug Stop
	   Register (DSR) */

	/* TODO - check if we have any software breakpoints in place before
	   setting this value - the kernel, for instance, relies on l.trap
	   instructions not stalling the processor! */
	debug_reg_list[OR1K_DEBUG_REG_DSR] |= OR1K_DSR_TE;

	/* write debug registers (starting from DMR1 register) */
	retval = or1k_jtag_write_cpu(&or1k->jtag, OR1K_DMR1_CPU_REG_ADD, OR1K_DEBUG_REG_NUM, debug_reg_list);
	if (retval != ERROR_OK)
		return retval;

	resume_pc = buf_get_u32(or1k->core_cache->reg_list[OR1K_REG_NPC].value,
				0, 32);
	/* the front-end may request us not to handle breakpoints */
	if (handle_breakpoints) {
		/* Single step past breakpoint at current address */
		breakpoint = breakpoint_find(target, resume_pc);
		if (breakpoint) {
			LOG_DEBUG("unset breakpoint at 0x%8.8" PRIx32 "", breakpoint->address);
#if 0
			/* Do appropriate things here to remove breakpoint. */
#endif
		}
	}
	/* Unstall time */

	/* Mohor debug if, clearing control register unstalls */
	retval = or1k_jtag_write_cpu_cr(&or1k->jtag, 0, 0);
	if (retval != ERROR_OK)
		return retval;

	if (step)
		target->debug_reason = DBG_REASON_SINGLESTEP;
	else
		target->debug_reason = DBG_REASON_NOTHALTED;

	/* registers are now invalid */
	register_cache_invalidate(or1k->core_cache);

	if (!debug_execution) {
		target->state = TARGET_RUNNING;
		target_call_event_callbacks(target, TARGET_EVENT_RESUMED);
		LOG_DEBUG("target resumed at 0x%" PRIx32 "", resume_pc);
	} else {
		target->state = TARGET_DEBUG_RUNNING;
		target_call_event_callbacks(target, TARGET_EVENT_DEBUG_RESUMED);
		LOG_DEBUG("target debug resumed at 0x%" PRIx32 "", resume_pc);
	}

	return ERROR_OK;
}

static int or1k_resume(struct target *target, int current,
		uint32_t address, int handle_breakpoints, int debug_execution)
{
	return or1k_resume_or_step(target, current, address,
				   handle_breakpoints,
				   debug_execution,
				   /* Single step? No. */
				   0);
}

static int or1k_step(struct target *target, int current,
		     uint32_t address, int handle_breakpoints)
{
	return or1k_resume_or_step(target, current, address,
				   handle_breakpoints,
				   /* TARGET_EVENT_RESUMED:
				      target resumed to execute user code */
				   0,
				   /* Single step? Yes. */
				   1);

}

static int or1k_add_breakpoint(struct target *target,
			       struct breakpoint *breakpoint)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int retval;

	LOG_DEBUG("Adding breakpoint: addr %08x, len %d, type %d, set: %d, id: %d",
		  breakpoint->address, breakpoint->length, breakpoint->type,
		  breakpoint->set, breakpoint->unique_id);

	/* Only support SW breakpoints for now. */
	if (breakpoint->type == BKPT_HARD)
		LOG_ERROR("HW breakpoints not supported for now. Doing SW breakpoint.");

	/* Read and save the instruction */
	or1k_jtag_read_memory32(&or1k->jtag,
				breakpoint->address ,
				1,
				(uint32_t *)breakpoint->orig_instr);

	/* Sub in the OR1K trap instruction */
	uint32_t or1k_trap_insn;
	/* set correct endianess */
	target_buffer_set_u32(target, (uint8_t *)&or1k_trap_insn, OR1K_TRAP_INSTR);
	or1k_jtag_write_memory32(&or1k->jtag,
				breakpoint->address ,
				 1,
				 (uint32_t *)&or1k_trap_insn);

	/* invalidate instruction cache */
	retval = or1k_jtag_write_cpu(&or1k->jtag,
			OR1K_ICBIR_CPU_REG_ADD, 1, &breakpoint->address);
	if (retval != ERROR_OK)
		return retval;

	return ERROR_OK;
}

static int or1k_remove_breakpoint(struct target *target,
				  struct breakpoint *breakpoint)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int retval;

	LOG_DEBUG("Removing breakpoint: addr %08x, len %d, type %d, set: %d, id: %d",
		  breakpoint->address, breakpoint->length, breakpoint->type,
		  breakpoint->set, breakpoint->unique_id);

	/* Only support SW breakpoints for now. */
	if (breakpoint->type == BKPT_HARD)
		LOG_ERROR("HW breakpoints not supported for now. Doing SW breakpoint.");

	/* Replace the removed instruction */
	or1k_jtag_write_memory32(&or1k->jtag,
				breakpoint->address ,
				 1,
				 (uint32_t *)breakpoint->orig_instr);

	/* invalidate instruction cache */
	retval = or1k_jtag_write_cpu(&or1k->jtag,
			OR1K_ICBIR_CPU_REG_ADD, 1, &breakpoint->address);
	if (retval != ERROR_OK)
		return retval;

	return ERROR_OK;
}

static int or1k_add_watchpoint(struct target *target,
			       struct watchpoint *watchpoint)
{
	LOG_ERROR("%s: implement me", __func__);
	return ERROR_OK;
}

static int or1k_remove_watchpoint(struct target *target,
				  struct watchpoint *watchpoint)
{
	LOG_ERROR("%s: implement me", __func__);
	return ERROR_OK;
}

static int or1k_bulk_read_memory(struct target *target, uint32_t address,
		uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);
	const unsigned int blocks_per_round = 1024; /* some resonable value */
	unsigned int blocks_this_round;

	/* Break it up into 4 byte blocks */
	uint32_t block_count_left = count;
	uint32_t block_count_address = address;
	uint8_t *block_count_buffer = (uint8_t *) buffer;

	/* Count is in 4-byte words */
	LOG_DEBUG("address 0x%x count %d", address, count);

	while (block_count_left) {
		blocks_this_round = (block_count_left > blocks_per_round) ?
			blocks_per_round : block_count_left;

		or1k_jtag_read_memory32(&or1k->jtag,
					 block_count_address ,
					 blocks_this_round,
					 (uint32_t *)block_count_buffer);

		block_count_left -= blocks_this_round;
		block_count_address += 4*blocks_per_round;
		block_count_buffer += 4*blocks_per_round;
	}

	return ERROR_OK;
}

static int or1k_bulk_write_memory(struct target *target, uint32_t address,
		uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);
	const unsigned int blocks_per_round = 1024; /* some reasonable value */
	unsigned int blocks_this_round;

	/* Break it up into 4 byte blocks */
	uint32_t block_count_left = count;
	uint32_t block_count_address = address;
	uint8_t *block_count_buffer = (uint8_t *) buffer;

	/* Count is in 4-byte words */
	LOG_DEBUG("address 0x%x count %d", address, count);

	while (block_count_left) {
		blocks_this_round = (block_count_left > blocks_per_round) ?
			blocks_per_round : block_count_left;

		or1k_jtag_write_memory32(&or1k->jtag,
					 block_count_address ,
					 blocks_this_round,
					 (uint32_t *)block_count_buffer);

		block_count_left -= blocks_this_round;
		block_count_address += 4*blocks_per_round;
		block_count_buffer += 4*blocks_per_round;
	}

	return ERROR_OK;
}

static int or1k_read_memory(struct target *target, uint32_t address,
		uint32_t size, uint32_t count, uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);

	LOG_DEBUG("address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32
		  ",count: 0x%8.8" PRIx32 "", address, size, count);

	if (target->state != TARGET_HALTED) {
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) ||
	    !(buffer))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if (((size == 4) && (address & 0x3u)) || ((size == 2) &&
						  (address & 0x1u)))
		return ERROR_TARGET_UNALIGNED_ACCESS;


	if (size == 4 && count > 1)
		return or1k_bulk_read_memory(target, address, count, buffer);

	switch (size) {
	case 4:
		return or1k_jtag_read_memory32(&or1k->jtag, address,
					       count,
					       (uint32_t *)(void *)buffer);
		break;
	case 2:
		return or1k_jtag_read_memory16(&or1k->jtag, address,
					       count,
					       (uint16_t *)(void *)buffer);
		break;
	case 1:
		return or1k_jtag_read_memory8(&or1k->jtag, address,
					      count,
					      buffer);
		break;
	default:
		break;
	}

	return ERROR_OK;
}

static int or1k_write_memory(struct target *target, uint32_t address,
		uint32_t size, uint32_t count, const uint8_t *buffer)
{
	struct or1k_common *or1k = target_to_or1k(target);

	LOG_DEBUG("address: 0x%8.8" PRIx32 ", size: 0x%8.8" PRIx32
		  ", count: 0x%8.8" PRIx32 "", address, size, count);

	if (target->state != TARGET_HALTED) {
		LOG_WARNING("target not halted");
		return ERROR_TARGET_NOT_HALTED;
	}

	/* sanitize arguments */
	if (((size != 4) && (size != 2) && (size != 1)) || (count == 0) ||
	    !(buffer))
		return ERROR_COMMAND_SYNTAX_ERROR;

	if (((size == 4) && (address & 0x3u)) || ((size == 2) &&
						  (address & 0x1u)))
		return ERROR_TARGET_UNALIGNED_ACCESS;

	if (size == 4 && count > 1)
		return or1k_bulk_write_memory(target, address, count, buffer);

	switch (size) {
	case 4:
		return or1k_jtag_write_memory32(&or1k->jtag, address, count,
						(uint32_t *)(void *)buffer);
		break;
	case 2:
		return or1k_jtag_write_memory16(&or1k->jtag, address, count,
						(uint16_t *)(void *)buffer);
		break;
	case 1:
		return or1k_jtag_write_memory8(&or1k->jtag, address, count,
					       buffer);
		break;
	default:
		break;
	}

	return ERROR_OK;
}


static int or1k_init_target(struct command_context *cmd_ctx,
		struct target *target)
{
	struct or1k_common *or1k = target_to_or1k(target);

	or1k->jtag.tap = target->tap;

	or1k_build_reg_cache(target);

	return ERROR_OK;
}

static int or1k_target_create(struct target *target, Jim_Interp *interp)
{
	struct or1k_common *or1k = calloc(1, sizeof(struct or1k_common));

	target->arch_info = or1k;

	or1k_create_reg_list(target);

	return ERROR_OK;
}

static int or1k_examine(struct target *target)
{
	uint32_t cpu_cr;
	struct or1k_common *or1k = target_to_or1k(target);

	if (!target_was_examined(target)) {
		target_set_examined(target);
		/* Do nothing special yet - Julius
		 */

		/* check for processor halted */

		/* Possible specific to Mohor debug interface - others may
		 * have to do something different here.
		 */
		or1k_jtag_read_cpu_cr(&or1k->jtag, &cpu_cr);
		if (cpu_cr & OR1K_MOHORDBGIF_CPU_CR_STALL) {
			LOG_DEBUG("target is halted");
			target->state = TARGET_HALTED;
		} else
			target->state = TARGET_RUNNING;
	}

	return ERROR_OK;
}

int or1k_arch_state(struct target *target)
{
	return ERROR_OK;
}

int or1k_get_gdb_reg_list(struct target *target, struct reg **reg_list[],
			  int *reg_list_size, int list_type)
{
	struct or1k_common *or1k = target_to_or1k(target);
	int i;

	if (list_type == G_REGISTERS_LIST) {
		/* We will have this called whenever GDB connects. */
		or1k_save_context(target);

		*reg_list_size = OR1KNUMCOREREGS;
		/* this is free()'d back in gdb_server.c's gdb_get_register_packet() */
		*reg_list = malloc((*reg_list_size) * sizeof(struct reg *));

		for (i = 0; i < OR1KNUMCOREREGS; i++)
			(*reg_list)[i] = &or1k->core_cache->reg_list[i];
	} else {
		*reg_list_size = or1k->nb_regs;
		*reg_list = malloc((*reg_list_size) * sizeof(struct reg *));

		for (i = 0; i < or1k->nb_regs; i++)
			(*reg_list)[i] = &or1k->core_cache->reg_list[i];
	}

	return ERROR_OK;

}

static int or1k_checksum_memory(struct target *target, uint32_t address,
		uint32_t count, uint32_t *checksum) {

	return ERROR_FAIL;

}

COMMAND_HANDLER(or1k_addreg_command_handler)
{
	struct target *target = get_current_target(CMD_CTX);
	struct or1k_core_reg new_reg;
	uint32_t addr;

	if (CMD_ARGC != 4)
		return ERROR_COMMAND_SYNTAX_ERROR;

	new_reg.target = NULL;
	new_reg.or1k_common = NULL;

	COMMAND_PARSE_NUMBER(u32, CMD_ARGV[1], addr);

	new_reg.name = strdup(CMD_ARGV[0]);
	new_reg.spr_num = addr;
	new_reg.feature = strdup(CMD_ARGV[2]);
	new_reg.group = strdup(CMD_ARGV[3]);

	or1k_add_reg(target, &new_reg);

	LOG_DEBUG("Add reg \"%s\" @ 0x%08X, group \"%s\", feature \"%s\"",
		  new_reg.name, addr, new_reg.group, new_reg.feature);

	return ERROR_OK;
}

static const struct command_registration or1k_reg_command_handlers[] = {
	{
		"addreg",
		.handler = or1k_addreg_command_handler,
		.mode = COMMAND_ANY,
		.usage = "addreg name addr feature group",
		.help = "add a register to the register list",
	},
	COMMAND_REGISTRATION_DONE
};

const struct command_registration or1k_command_handlers[] = {
	{
		.chain = or1k_reg_command_handlers,
	},
	COMMAND_REGISTRATION_DONE
};


struct target_type or1k_target = {
	.name = "or1k",

	.poll = or1k_poll,
	.arch_state = or1k_arch_state,

	.target_request_data = NULL,

	.halt = or1k_halt,
	.resume = or1k_resume,
	.step = or1k_step,

	.assert_reset = or1k_assert_reset,
	.deassert_reset = or1k_deassert_reset,
	.soft_reset_halt = or1k_soft_reset_halt,

	.get_gdb_reg_list = or1k_get_gdb_reg_list,

	.read_memory = or1k_read_memory,
	.write_memory = or1k_write_memory,
	.bulk_write_memory = or1k_bulk_write_memory,
	.checksum_memory = or1k_checksum_memory,

	.generate_tdesc_file = or1k_generate_tdesc,

	.commands = or1k_command_handlers,
	.add_breakpoint = or1k_add_breakpoint,
	.remove_breakpoint = or1k_remove_breakpoint,
	.add_watchpoint = or1k_add_watchpoint,
	.remove_watchpoint = or1k_remove_watchpoint,

	.target_create = or1k_target_create,
	.init_target = or1k_init_target,
	.examine = or1k_examine,
};

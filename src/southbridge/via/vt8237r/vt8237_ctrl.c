/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2008 Rudolf Marek <r.marek@assembler.cz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License v2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <device/device.h>
#include <device/pci.h>
#include <device/pci_ops.h>
#include <device/pci_ids.h>
#include <console/console.h>

/* We support here K8M890/K8T890 and VT8237/S/A PCI1/Vlink */

static void vt8237_cfg(struct device *dev)
{
	u8 regm, regm2, regm3;

	device_t devfun3;

	devfun3 = dev_find_device(PCI_VENDOR_ID_VIA,
					   PCI_DEVICE_ID_VIA_K8T890CE_3, 0);

		if (!devfun3)
			devfun3 = dev_find_device(PCI_VENDOR_ID_VIA,
					   PCI_DEVICE_ID_VIA_K8M890CE_3, 0);

		if (!devfun3)
			die("Unknown NB");

	/* CPU to PCI Flow Control 1 & 2, just fill in recommended */
	pci_write_config8(dev, 0x70, 0xc2);
	pci_write_config8(dev, 0x71, 0xc8);

	/* PCI Control */
	pci_write_config8(dev, 0x72, 0xee);
	pci_write_config8(dev, 0x73, 0x01);
	pci_write_config8(dev, 0x74, 0x3c);
	pci_write_config8(dev, 0x75, 0x0f);
	pci_write_config8(dev, 0x76, 0x50);
	pci_write_config8(dev, 0x77, 0x48);
	pci_write_config8(dev, 0x78, 0x01);
	/* APIC on HT */
	pci_write_config8(dev, 0x7c, 0x77); //maybe Enable LDT APIC Mode bit3 set to 1

	/* WARNING: Need to copy some registers from NB (D0F3) to SB (D11F7). */

	regm = pci_read_config8(devfun3, 0x88);	/* Shadow mem CTRL */
	pci_write_config8(dev, 0x57, regm);

	regm = pci_read_config8(devfun3, 0x80);	/* Shadow page C */
	pci_write_config8(dev, 0x61, regm);

	regm = pci_read_config8(devfun3, 0x81);	/* Shadow page D */
	pci_write_config8(dev, 0x62, regm);

	/* Shadow page F + memhole copy */
	regm = pci_read_config8(devfun3, 0x83);
	pci_write_config8(dev, 0x63, regm);

	regm3 = pci_read_config8(devfun3, 0x82);/* Shadow page E */
	pci_write_config8(dev, 0x64, regm);

	regm = pci_read_config8(devfun3, 0x86);	/* SMM and APIC decoding */
	pci_write_config8(dev, 0xe6, regm);
}

/**
 * Example of setup: Setup the V-Link for VT8237R, 8X mode.
 *
 * For K8T890CF VIA recommends what is in VIA column, AW is award 8X:
 *
 *						 REG   DEF   AW  VIA-8X VIA-4X
 *						 -----------------------------
 * NB V-Link Manual Driving Control strobe	 0xb5  0x46  0x46  0x88  0x88
 * NB V-Link Manual Driving Control - Data	 0xb6  0x46  0x46  0x88  0x88
 * NB V-Link Receiving Strobe Delay		 0xb7  0x02  0x02  0x61  0x01
 * NB V-Link Compensation Control bit4,0 (b5,b6) 0xb4  0x10  0x10  0x11  0x11
 * SB V-Link Strobe Drive Control 		 0xb9  0x00  0xa5  0x98  0x98
 * SB V-Link Data drive Control????		 0xba  0x00  0xbb  0x77  0x77
 * SB V-Link Receive Strobe Delay????		 0xbb  0x04  0x11  0x11  0x11
 * SB V-Link Compensation Control bit0 (use b9)	 0xb8  0x00  0x01  0x01  0x01
 * V-Link CKG Control				 0xb0  0x05  0x05  0x06  0x03
 * V-Link CKG Control				 0xb1  0x05  0x05  0x01  0x03
 */

static void vt8237s_vlink_init(struct device *dev)
{
	u8 reg;

	device_t devfun7;

	devfun7 = dev_find_device(PCI_VENDOR_ID_VIA,
					   PCI_DEVICE_ID_VIA_K8T890CE_7, 0);

	if (!devfun7)
		devfun7 = dev_find_device(PCI_VENDOR_ID_VIA,
					   PCI_DEVICE_ID_VIA_K8M890CE_7, 0);

	/* no pairing NB found */
	if (!devfun7)
		return;

	/*
	 * This init code is valid only for the VT8237S! For different
	 * sounthbridges (e.g. VT8237A, VT8237S, VT8237R (without plus R)
	 * and VT8251) a different init code is required.
	 */

	pci_write_config8(devfun7, 0xb5, 0x66);
	pci_write_config8(devfun7, 0xb6, 0x66);
	pci_write_config8(devfun7, 0xb7, 0x65);

	reg = pci_read_config8(devfun7, 0xb4);
	reg |= 0x1;
	pci_write_config8(devfun7, 0xb4, reg);

	pci_write_config8(dev, 0xb9, 0x68);
	pci_write_config8(dev, 0xba, 0x88);
	pci_write_config8(dev, 0xbb, 0x89);


	reg = pci_read_config8(dev, 0xbd);
	reg |= 0x3;
	pci_write_config8(dev, 0xbd, reg);

	/* Program V-link 8X 8bit full duplex, parity disabled FIXME */
	pci_write_config8(dev, 0x48, 0x13);
}

static void ctrl_enable(struct device *dev) {
	
	/* enable the 0:13 and 0:13.1 */
	/* FIXME */
	pci_write_config8(dev, 0x4f, 0x43);
}


extern void dump_south(device_t dev);

static void ctrl_init(struct device *dev) {

	/* TODO: Fix some ordering issue fo V-link set Rx77[6] and PCI1_Rx4F[0]
	   should to 1 FIXME DO you need?*/

	/* VT8237R specific configuration  other SB are done in their own directories */
	/*  add A version */
	device_t devsb = dev_find_device(PCI_VENDOR_ID_VIA,
					PCI_DEVICE_ID_VIA_VT8237S_LPC, 0);
	if (devsb) {
		/* FIXME: Skip v-link setup for now */
//		vt8237s_vlink_init(dev);
	}

	/* configure PCI1 and copy mirror registers from D0F3 */
	vt8237_cfg(dev);
	dump_south(dev);
}

static const struct device_operations ctrl_ops = {
	.read_resources		= pci_dev_read_resources,
	.set_resources		= pci_dev_set_resources,
	.enable_resources	= pci_dev_enable_resources,
	.init			= ctrl_init,
	.enable			= ctrl_enable,
	.ops_pci		= 0,
};

static const struct pci_driver northbridge_driver_t __pci_driver = {
	.ops	= &ctrl_ops,
	.vendor	= PCI_VENDOR_ID_VIA,
	.device	= PCI_DEVICE_ID_VIA_VT8237_VLINK,
};

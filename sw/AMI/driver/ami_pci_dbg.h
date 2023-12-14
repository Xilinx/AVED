// SPDX-License-Identifier: GPL-2.0-only
/*
 * ami_pci_dbg.h - This file contains PCI debug/printing definitions.
 *
 * Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
 */

#ifndef AMI_PCI_DBG_H
#define AMI_PCI_DBG_H

#include <linux/device.h>

#include "ami_pcie.h"
#include "ami_vsec.h"

/* PCIe DEBUG */

char *pcie_vendor_id_to_str(uint16_t id);
void print_pcie_vendor_id(struct pci_dev *dev, uint16_t id);

char *pcie_device_id_to_str(uint16_t id);
void print_pcie_device_id(struct pci_dev *dev, uint16_t id);

void print_pcie_command(struct pci_dev *dev, uint16_t command);

void print_pcie_status(struct pci_dev *dev, uint16_t status);

void print_pcie_class_revision(struct pci_dev *dev, uint32_t class_revision);

void print_pcie_bar_info(struct pci_dev *dev, int bar_idx, struct bar_header_struct bar);

void print_pcie_cap_power_management(struct pci_dev *dev, uint16_t cap_pm);
void print_pcie_cap_pm_pme_support(struct pci_dev *dev, uint16_t cap_pm_pme_support);

void print_pcie_cap_pm_status_cntrl(struct pci_dev *dev, uint16_t pm_status_cntrl);

void print_pcie_cap_pm_cntrl_bridge_ext(struct pci_dev *dev, uint16_t pm_cntrl_bridge_ext);

void print_pcie_cap_pm_data(struct pci_dev *dev, uint16_t pm_data);

void print_pcie_cap_msi(struct pci_dev *dev, uint16_t msi_config_status, uint16_t msi_msg_addr, uint16_t msi_msg_data, uint32_t msi_msg_uppr_addr);

void print_pcie_cap_cap(struct pci_dev *dev, uint16_t cap);

void print_pcie_cap_dev(struct pci_dev *dev, uint32_t dev_cap);

void print_pcie_cap_dev_ctrl(struct pci_dev *dev, uint16_t dev_ctrl);

void print_pcie_cap_dev_status( struct pci_dev *dev, uint16_t dev_stat);

void print_pcie_cap_link_cap(struct pci_dev *dev, uint32_t link_cap);

void print_pcie_cap_link_ctrl(struct pci_dev *dev, uint16_t link_ctrl);

void print_pcie_cap_link_status(struct pci_dev *dev, uint16_t link_stat);

void print_pcie_cap_slot_cap(struct pci_dev *dev, uint32_t slot_cap);

void print_pcie_cap_slot_ctrl(struct pci_dev *dev, uint16_t slot_ctrl);

void print_pcie_cap_slot_status(struct pci_dev *dev, uint16_t slot_stat);

void print_pcie_cap_root_ctrl(struct pci_dev *dev, uint16_t root_ctrl);

void print_pcie_cap_root_cap(struct pci_dev *dev, uint16_t root_cap);

void print_pcie_cap_root_status(struct pci_dev *dev, uint32_t root_stat);

void print_pcie_cap_dev_cap_2(struct pci_dev *dev, uint32_t dev_cap);

void print_pcie_cap_dev_ctrl_2( struct pci_dev *dev, uint16_t dev_ctrl);

void print_pcie_cap_link_cap_2(struct pci_dev *dev, uint32_t link_cap);

void print_pcie_cap_link_ctrl_2(struct pci_dev *dev, uint16_t link_ctrl);

void print_pcie_cap_link_status_2(struct pci_dev *dev, uint16_t link_stat);

#ifdef VERBOSE_DEBUG
void find_pci_ext_capability(struct pci_dev *dev, char *cap_name, int cap);
void pcie_find_supported_ext_caps(struct pci_dev *dev);
#endif

#ifdef VERBOSE_DEBUG
void find_pci_capability(struct pci_dev *dev, char *cap_name, int cap);
void pcie_find_supported_cap(struct pci_dev *dev);
#endif

void print_pcie_stat(struct pci_dev *dev, pcie_cap_struct *cap);


/* VSEC DEBUG */

void print_endpoint_info(struct pci_dev *dev, endpoint_info_struct endpoint_info);

#endif  /* AMI_PCI_DBG_H */

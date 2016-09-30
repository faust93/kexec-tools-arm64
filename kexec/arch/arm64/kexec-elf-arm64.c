/*
 * ARM64 kexec elf support.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <linux/elf.h>

#include "kexec-arm64.h"
#include "kexec-elf.h"
#include "kexec-syscall.h"

int elf_arm64_probe(const char *kernel_buf, off_t kernel_size)
{
	struct mem_ehdr ehdr;
	int result;

	result = build_elf_exec_info(kernel_buf, kernel_size, &ehdr, 0);

	if (result < 0) {
		dbgprintf("%s: Not an ELF executable.\n", __func__);
		goto on_exit;
	}

	if (ehdr.e_machine != EM_AARCH64) {
		dbgprintf("%s: Not an AARCH64 ELF executable.\n", __func__);
		result = -1;
		goto on_exit;
	}

	result = 0;
on_exit:
	free_elf_info(&ehdr);
	return result;
}

int elf_arm64_load(int argc, char **argv, const char *kernel_buf,
	off_t kernel_size, const char *kernel_compressed_buf,
	off_t kernel_compressed_size, struct kexec_info *info)
{
	struct mem_ehdr ehdr;
	int result;
	int i;

	if (info->kexec_flags & KEXEC_ON_CRASH) {
		fprintf(stderr, "kexec: kdump not yet supported on arm64\n");
		return -EFAILED;
	}

	result = build_elf_exec_info(kernel_buf, kernel_size, &ehdr, 0);

	if (result < 0) {
		dbgprintf("%s: build_elf_exec_info failed\n", __func__);
		goto exit;
	}

	/* Find and process the arm64 image header. */

	for (i = 0; i < ehdr.e_phnum; i++) {
		struct mem_phdr *phdr = &ehdr.e_phdr[i];
		const struct arm64_image_header *h;
		unsigned long header_offset;

		if (phdr->p_type != PT_LOAD)
			continue;

		/*
		 * When CONFIG_ARM64_RANDOMIZE_TEXT_OFFSET=y the image header
		 * could be offset in the elf segment.  The linker script sets
		 * ehdr.e_entry to the start of text.
		 */

		header_offset = ehdr.e_entry - phdr->p_vaddr;

		h = (const struct arm64_image_header *)(
			kernel_buf + phdr->p_offset + header_offset);

		if (arm64_process_image_header(h))
			continue;

		arm64_mem.vp_offset = ehdr.e_entry - arm64_mem.text_offset;

		dbgprintf("%s: e_entry:       %016llx -> %016lx\n", __func__,
			ehdr.e_entry,
			virt_to_phys(ehdr.e_entry));
		dbgprintf("%s: p_vaddr:       %016llx -> %016lx\n", __func__,
			phdr->p_vaddr,
			virt_to_phys(phdr->p_vaddr));
		dbgprintf("%s: header_offset: %016lx\n", __func__,
			header_offset);
		dbgprintf("%s: text_offset:   %016lx\n", __func__,
			arm64_mem.text_offset);
		dbgprintf("%s: image_size:    %016lx\n", __func__,
			arm64_mem.image_size);
		dbgprintf("%s: phys_offset:   %016lx\n", __func__,
			arm64_mem.phys_offset);
		dbgprintf("%s: vp_offset:     %016lx\n", __func__,
			arm64_mem.vp_offset);
		dbgprintf("%s: PE format:     %s\n", __func__,
			(arm64_header_check_pe_sig(h) ? "yes" : "no"));

		result = elf_exec_load(&ehdr, info);

		if (result) {
			dbgprintf("%s: elf_exec_load failed\n", __func__);
			goto exit;
		}

		result = arm64_load_other_segments(info,
			virt_to_phys(ehdr.e_entry), kernel_buf, kernel_size);
		goto exit;
	}

	dbgprintf("%s: Bad arm64 image header\n", __func__);
	result = -EFAILED;
	goto exit;

exit:
	reset_vp_offset();
	free_elf_info(&ehdr);
	if (result)
		fprintf(stderr, "kexec: Bad elf image file, load failed.\n");
	return result;
}

void elf_arm64_usage(void)
{
	printf(
"     An ARM64 ELF image, big or little endian.\n"
"     Typically vmlinux or a stripped version of vmlinux.\n\n");
}

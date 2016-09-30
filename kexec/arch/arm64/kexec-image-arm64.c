/*
 * ARM64 kexec binary image support.
 */

#define _GNU_SOURCE
#include "kexec-arm64.h"

int image_arm64_probe(const char *kernel_buf, off_t kernel_size)
{
	const struct arm64_image_header *h;

	if (kernel_size < sizeof(struct arm64_image_header)) {
		dbgprintf("%s: No arm64 image header.\n", __func__);
		return -1;
	}

	h = (const struct arm64_image_header *)(kernel_buf);

	if (!arm64_header_check_magic(h)) {
		dbgprintf("%s: Bad arm64 image header.\n", __func__);
		return -1;
	}

	return 0;
}

int image_arm64_load(int argc, char **argv, const char *kernel_buf,
	off_t kernel_size, const char *kernel_compressed_buf,
	off_t kernel_compressed_size, struct kexec_info *info)
{
	const struct arm64_image_header *h;
	unsigned long image_base;

	h = (const struct arm64_image_header *)(kernel_buf);

	if (arm64_process_image_header(h))
		return -1;

	dbgprintf("%s: text_offset:   %016lx\n", __func__,
		arm64_mem.text_offset);
	dbgprintf("%s: image_size:    %016lx\n", __func__,
		arm64_mem.image_size);
	dbgprintf("%s: phys_offset:   %016lx\n", __func__,
		arm64_mem.phys_offset);
	dbgprintf("%s: PE format:     %s\n", __func__,
		(arm64_header_check_pe_sig(h) ? "yes" : "no"));

	image_base = get_phys_offset() + arm64_mem.text_offset;
	
	add_segment_phys_virt(info, kernel_buf, kernel_size, image_base,
		arm64_mem.image_size, 0);

	return arm64_load_other_segments(info, image_base,
		kernel_compressed_buf, kernel_compressed_size);
}

void image_arm64_usage(void)
{
	printf(
"     An ARM64 binary image, compressed or not, big or little endian.\n"
"     Typically an Image, Image.gz or Image.lzma file.\n\n");
}

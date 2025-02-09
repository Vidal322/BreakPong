#include <lcom/lcf.h>
#include "graphics.h"

int (set_graphics_mode)(uint16_t mode) {
  reg86_t reg86; // struct that supports instructions
  memset(&reg86, 0, sizeof(reg86)); // puts everything 0
  reg86.intno = 0x10; // it's always 0x10
  reg86.ah = 0x4F; // most significant part of AX
  reg86.al = 0x02; // least significant part of AX that is 0x02 in graphic mode
  reg86.bx = mode | BIT(14); // determines the mode with linear frame buffer
  if (sys_int86(&reg86) != OK) {
    printf("Error exiting graphics mode");
    return !OK;
  }
  return OK;
}

int (set_text_mode)() {
  reg86_t reg86;
  memset(&reg86 , 0, sizeof(reg86));
  reg86.intno = 0x10;
  reg86.ah = 0x00; 
  reg86.al = 0x03; // 0x03 is the text mode
  reg86.bx = 0x00; // there is no need to set the mode
  if (sys_int86(&reg86) != 0) {
    printf("Set text mode failed");
    return !OK;
  }
  return OK;
}

int (set_frame_buffer)(uint16_t mode) {
  
  memset(&modeinfo, 0, sizeof(modeinfo)); // puts everything 0
  if (vbe_get_mode_info(mode, &modeinfo) != OK) { // gets the mode info
    printf("Error getting mode info");
    return !OK;
  }

  // We have to calculate the size of the frame buffer
  unsigned int bytes_per_pixel = (modeinfo.BitsPerPixel + 7) / 8; // adds 7 to the bits per pixel and divides by 8 because we want the number of bytes

  unsigned int frame_size = modeinfo.XResolution * modeinfo.YResolution * bytes_per_pixel; // calculates the size of the frame buffer in bytes

  // We have to map the frame buffer
  struct minix_mem_range physical_memory; // struct that supports memory ranges
  physical_memory.mr_base = modeinfo.PhysBasePtr; // the base address of the frame buffer
  physical_memory.mr_limit = physical_memory.mr_base + frame_size; // the limit address of the frame buffer

  // Allocates the memory range
  if (sys_privctl(SELF, SYS_PRIV_ADD_MEM, &physical_memory) != OK) {
    printf("Error allocating memory range");
    return !OK;
  }

  // virtual address of the frame buffer
  frame_buffer = vm_map_phys(SELF, (void*)physical_memory.mr_base, frame_size);

  if (frame_buffer == NULL) {
    printf("Error mapping the frame buffer");
    return !OK;

  }
  return OK;
}

int (vg_draw_pixel)(uint16_t x, uint16_t y, uint32_t color) {
  if (x > modeinfo.XResolution || y > modeinfo.YResolution) {
    printf("Error: pixel out of bounds");
    return !OK;
  }

  unsigned bytes_per_pixel = (modeinfo.BitsPerPixel + 7) / 8;
  unsigned int index = (modeinfo.XResolution * y + x) * bytes_per_pixel; // calculates the index of the pixel

  if (memcpy(&frame_buffer[index], &color, bytes_per_pixel) == NULL) {
    printf("Error copying the pixel");
    return !OK;
  }
  return OK;
}

int (vg_draw_hline)(uint16_t x, uint16_t y, uint16_t len, uint32_t color) {
  for (unsigned i = 0; i < len; i++) {
    if (vg_draw_pixel(x + i, y, color) != OK) {
      printf("Error drawing the pixel");
      return !OK;
    }
  }
  return OK;
}

int (vg_draw_rectangle)(uint16_t x, uint16_t
 y, uint16_t width, uint16_t height, uint32_t color) {
  for (unsigned i = 0; i < height; i++) {
    if (vg_draw_hline(x, y + i, width, color) != OK) {
      printf("Error during rectangle drawing");
      vg_exit();
      return !OK;
    }
  }
  return OK;
 }

int (print_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  xpm_image_t img;
  uint8_t * colors = xpm_load(xpm, XPM_INDEXED, &img);

  for (int h = 0; h < img.height; h++) {
    for (int w = 0; w < img.width; w++) {
      if (vg_draw_pixel(x + w, y + h, *colors) != OK) return !OK;
      colors++;
    }
  }
  return OK;
}

int (normalize_color)(uint32_t color, uint32_t *new_color) {
  if (modeinfo.BitsPerPixel == 32) *new_color = color;
  else *new_color = color & (BIT(modeinfo.BitsPerPixel) - 1);
  return OK;
}

uint32_t (direct_mode)(uint32_t R, uint32_t G, uint32_t B) {
  return (R << modeinfo.RedFieldPosition) | (G << modeinfo.GreenFieldPosition) | (B << modeinfo.BlueFieldPosition);
}

uint32_t (indexed_mode)(uint16_t col, uint16_t row, uint8_t step, uint32_t first, uint8_t n) {
  return (first + (row * n + col) * step) % (1 << modeinfo.BitsPerPixel);
}

uint32_t (Red)(unsigned j, uint8_t step, uint32_t first) {
  return (R(first) + j * step) % (1 << modeinfo.RedMaskSize);
}

uint32_t (Green)(unsigned i, uint8_t step, uint32_t first) {
  return (G(first) + i * step) % (1 << modeinfo.GreenMaskSize);
}

uint32_t (Blue)(unsigned j, unsigned i, uint8_t step, uint32_t first) {
  return (B(first) + (i + j) * step) % (1 << modeinfo.BlueMaskSize);
}

uint32_t (R)(uint32_t first){
  return ((1 << modeinfo.RedMaskSize) - 1) & (first >> modeinfo.RedFieldPosition);
}

uint32_t (G)(uint32_t first){
  return ((1 << modeinfo.GreenMaskSize) - 1) & (first >> modeinfo.GreenFieldPosition);
}

uint32_t (B)(uint32_t first){
  return ((1 << modeinfo.BlueMaskSize) - 1) & (first >> modeinfo.BlueFieldPosition);
}


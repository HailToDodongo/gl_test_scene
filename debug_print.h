
static void debug_print_screen(float x, float y, const char* str)
{
  int width = 8;
  int height = 12;
  int s = 0;

  while(*str) {
    char c = *str;
    if(c != ' ' && c != '\n')
    {
           if(c >= 'A' && c <= '_')s = (c - 'A' + 27) * width;
      else if(c >= 'a' && c <= 'z')s = (c - 'a' + 27+31) * width;
      else if(c >= '!' && c <= '@')s = (c - '!') * width;
      rdpq_texture_rectangle_raw(TILE0, x, y, x+width, y+height, s, 0, 1, 1);
    }
    ++str;
    x += 8;
  }
}

static void debug_printf_screen(float x, float y, const char* str, ...)
{
    char buf[512];
    size_t n = sizeof(buf);

    va_list va;
    va_start(va, str);
    char *buf2 = vasnprintf(buf, &n, str, va);
    va_end(va);

    debug_print_screen(x, y, buf2);
}

#define t3d_debug_print debug_print_screen
#define t3d_debug_printf debug_printf_screen
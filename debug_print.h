#define RCP_TICKS_TO_USECS(ticks) (((ticks) * 1000000ULL) / RCP_FREQUENCY)
#define PERCENT(fraction, total) ((total) > 0 ? (float)(fraction) * 100.0f / (float)(total) : 0.0f)

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

static void rspq_profile_dump_overlay_screen(size_t index, uint64_t frame_avg, const char *name, float *posY)
{
    uint64_t mean = profile_data.slots[index].total_ticks / profile_data.frame_count;
    uint64_t mean_us = RCP_TICKS_TO_USECS(mean);
    float relative = PERCENT(mean, frame_avg);

    char buf[64];
    sprintf(buf, "%3.2f%%", relative);
    
    debug_printf_screen(24, *posY, "%-10.10s %6llu %7llu# %8s",
        name,
        profile_data.slots[index].sample_count / profile_data.frame_count,
        mean_us,
        buf);

    *posY += 10;
}

void rspq_profile_dump_screen()
{
    if (profile_data.frame_count == 0)
        return;

    uint64_t frame_avg = profile_data.total_ticks / profile_data.frame_count;
    uint64_t frame_avg_us = RCP_TICKS_TO_USECS(frame_avg);

    uint64_t counted_time = 0;
    for (size_t i = 0; i < RSPQ_PROFILE_SLOT_COUNT; i++) counted_time += profile_data.slots[i].total_ticks;

    // The counted time could be slightly larger than the total time due to various measurement errors
    uint64_t overhead_time = profile_data.total_ticks > counted_time ? profile_data.total_ticks - counted_time : 0;
    uint64_t overhead_avg = overhead_time / profile_data.frame_count;
    uint64_t overhead_us = RCP_TICKS_TO_USECS(overhead_avg);

    float overhead_relative = PERCENT(overhead_avg, frame_avg);

    uint64_t rdp_busy_avg = profile_data.rdp_busy_ticks / profile_data.frame_count;
    uint64_t rdp_busy_us = RCP_TICKS_TO_USECS(rdp_busy_avg);
    float rdp_utilisation = PERCENT(rdp_busy_avg, frame_avg);

    float posY = 20;
    for (size_t i = 0; i < RSPQ_PROFILE_SLOT_COUNT; i++)
    {
        if (profile_data.slots[i].name == NULL)
            continue;

        rspq_profile_dump_overlay_screen(i, frame_avg, profile_data.slots[i].name, &posY);
    }
    float posYInc = 10;
    posY += posYInc;
    debug_printf_screen(24, posY, "Frames   : %7lld", profile_data.frame_count); posY += posYInc;
    debug_printf_screen(24, posY, "FPS      : %7.1f", (float)RCP_FREQUENCY/(float)frame_avg); posY += posYInc;
    debug_printf_screen(24, posY, "Avg frame: %7lld#", frame_avg_us); posY += posYInc;
    debug_printf_screen(24, posY, "RDP busy : %7lld# (%2.2f%%)", rdp_busy_us, rdp_utilisation); posY += posYInc;
    debug_printf_screen(24, posY, "Unrec.   : %7lld# (%2.2f%%)", overhead_us, overhead_relative);
}

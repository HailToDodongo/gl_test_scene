#include <libdragon.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>
#include <rspq_profile.h>

#include <malloc.h>
#include <math.h>
#include <stdarg.h>

static GLuint texture;

static GLfloat environment_color[] = { 0.5f, 0.5f, 0.6f, 1.f };
static GLfloat light_pos[4]        = { 0, 0, 0, 0 };
static GLfloat light_diffuse[4]    = { 1.0f, 1.0f, 1.0f, 1.0f };

static rspq_profile_data_t profile_data;
#include "debug_print.h"

int main()
{
	debug_init_isviewer();
	debug_init_usblog();
    dfs_init(DFS_DEFAULT_LOCATION);

    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS);

    rdpq_init();
    gl_init();

    rspq_profile_start();
    profile_data.frame_count = 0;

    surface_t zbuffer = surface_alloc(FMT_RGBA16, display_get_width(), display_get_height());

    sprite_t *sprite = sprite_load("rom:/unit1m.i8.sprite");
    sprite_t *spriteFont = sprite_load("rom:/font.ia4.sprite");
    sprite_t *spriteLogo = sprite_load("rom:/logo.ia8.sprite");
    model64_t *model = model64_load("rom:/scene.model64");

    glMatrixMode(GL_PROJECTION);
    gluPerspective(85.0f, (float)display_get_width() / (float)display_get_height(), 0.1f, 40.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // No Fog
    glEnable(GL_FOG);
    glFogf(GL_FOG_START, 3);
    glFogf(GL_FOG_END, 15);
    glFogfv(GL_FOG_COLOR, environment_color);

    // 1 ambient + 1 directional light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, environment_color);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    GLfloat mat_diffuse[] = { 0.8f, 0.8f, 1.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_diffuse);

    // Load Texture
    glEnable(GL_MULTISAMPLE_ARB);
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSpriteTextureN64(GL_TEXTURE_2D, sprite, &(rdpq_texparms_t){.s.repeats = REPEAT_INFINITE, .t.repeats = REPEAT_INFINITE});

    joypad_init();

    float cameraPos[3];
    float camRotX = 1.54f;
    float camRotY = 4.05f;

    float time = 0.0f;
    float camScale = 0.051812f;

    bool requestDisplayMetrics = false;
    bool displayMetrics = false;
    uint32_t dpl = 0;

    for(int frame = 0;; ++frame)
    {
        joypad_poll();
        joypad_inputs_t inputs = joypad_get_inputs(JOYPAD_PORT_1);

        if(inputs.btn.b) {
            requestDisplayMetrics = true;
        } else {
            requestDisplayMetrics = false;
            displayMetrics = false;
        }

        // Camera movement
        float x = inputs.stick_x / 128.f;
        float y = inputs.stick_y / 128.f;
        if (fabsf(y) > 0.01f)camScale -= y * 0.002f;
        if (fabsf(x) > 0.01f)camRotX += x * 0.2f;

        cameraPos[0] = 4.47f * camScale;
        cameraPos[1] = 134.180f * camScale;
        cameraPos[2] = 124.243f * camScale;

        // Draw
        surface_t *disp = display_get();
        rdpq_attach(disp, &zbuffer);

        gl_context_begin();

        glClearColor(0.12f, 0.0f, 0.06f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            cameraPos[0], cameraPos[1], cameraPos[2],
            cameraPos[0] + fm_cosf(camRotX) * fm_cosf(camRotY),
            cameraPos[1] + fm_sinf(camRotY),
            cameraPos[2] + fm_sinf(camRotX) * fm_cosf(camRotY),
            0, 1, 0
        );
        
        // Update rotating directional light
        light_pos[0] = fm_cosf(time * 10.2f);
        light_pos[1] = 0.0f;
        light_pos[2] = fm_sinf(time * 10.2f);
        glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

        if(dpl == 0) {
            dpl = glGenLists(1);
            glNewList(dpl, GL_COMPILE);

            glEnable(GL_LIGHTING);
            glEnable(GL_NORMALIZE);
            glEnable(GL_DEPTH_TEST);
        
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, texture);

            glEnable(GL_COLOR_MATERIAL);
            glEnable(GL_CULL_FACE);
            
            model64_draw(model);
            glEndList();
        }

        glCallList(dpl);
        
        gl_context_end();
        
        if(displayMetrics) // dumps profiling data to the screen
        {
            rdpq_sync_pipe();
            rdpq_sync_load();

            rdpq_set_mode_standard();
            rdpq_mode_antialias(AA_NONE);
            rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
            rdpq_mode_combiner(RDPQ_COMBINER1((TEX0,0,PRIM,0), (TEX0,0,PRIM,0)));
            rdpq_mode_alphacompare(1);

            rdpq_sprite_upload(TILE0, spriteFont, NULL);

            debug_printf_screen(140, 218, "FPS: %.4f", display_get_fps());
            rspq_profile_dump_screen();

            rdpq_mode_combiner(RDPQ_COMBINER_TEX);
            rdpq_mode_blender(RDPQ_BLENDER_MULTIPLY);
            rdpq_sprite_blit(spriteLogo, 24.0f, 206.0f, NULL);
        }

        rdpq_detach_show();
        rspq_profile_next_frame();

        if(frame == 30)
        {
            // we only want to update metrics if the overlay is not visible and vice-versa
            // this prevents noise introduced by the overlay itself
            if(!displayMetrics) {
                rspq_wait();
                rspq_profile_get_data(&profile_data);
                if(requestDisplayMetrics)displayMetrics = true;
            }

            frame = 0;
            rspq_profile_reset();
        }

        time += 1.0f / 60.0f;
    }
}

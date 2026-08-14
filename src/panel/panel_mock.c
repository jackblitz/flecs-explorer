#include "flecs_explorer/panel/panel_mock.h"

#include <stdio.h>
#include <string.h>

/**
 * Populates a MockWorldData structure with realistic Flecs ECS data.
 *
 * Provides simulated telemetry metrics, entity hierarchy tree, and JSON
 * component values for visual UI testing.
 *
 * @param outData Pointer to MockWorldData struct to initialize.
 */
void PanelMockWorldDataInit(MockWorldData *outData)
{
    if (outData == NULL) {
        return;
    }

    memset(outData, 0, sizeof(MockWorldData));

    snprintf(outData->worldName, sizeof(outData->worldName),
             "flecs_game_world");
    snprintf(outData->serverUrl, sizeof(outData->serverUrl),
             "http://localhost:2772");
    outData->fps = 60.0;
    outData->entityCount = 1420;
    outData->tableCount = 48;
    outData->systemCount = 12;
    outData->latencyMs = 1;
    outData->status = "ONLINE";
    outData->selectedEntityIndex = 0;
    outData->totalEntities = 7;

    // Entity 0: Player
    MockEntity *e0 = &outData->entities[0];
    e0->id = 0x1000;
    snprintf(e0->name, sizeof(e0->name), "Player");
    snprintf(e0->path, sizeof(e0->path), "game.actors.player");
    e0->componentCount = 4;
    snprintf(e0->components[0].name, sizeof(e0->components[0].name),
             "Position");
    snprintf(e0->components[0].jsonValue, sizeof(e0->components[0].jsonValue),
             "{\"x\": 12.5, \"y\": 0.0, \"z\": -4.2}");
    snprintf(e0->components[1].name, sizeof(e0->components[1].name),
             "Velocity");
    snprintf(e0->components[1].jsonValue, sizeof(e0->components[1].jsonValue),
             "{\"dx\": 1.0, \"dy\": 0.0, \"dz\": 0.0}");
    snprintf(e0->components[2].name, sizeof(e0->components[2].name), "Health");
    snprintf(e0->components[2].jsonValue, sizeof(e0->components[2].jsonValue),
             "{\"current\": 100, \"max\": 100}");
    snprintf(e0->components[3].name, sizeof(e0->components[3].name),
             "PlayerTag");
    snprintf(e0->components[3].jsonValue, sizeof(e0->components[3].jsonValue),
             "{}");

    // Entity 1: Enemy_01
    MockEntity *e1 = &outData->entities[1];
    e1->id = 0x1001;
    snprintf(e1->name, sizeof(e1->name), "Enemy_01");
    snprintf(e1->path, sizeof(e1->path), "game.actors.enemies.enemy_01");
    e1->componentCount = 4;
    snprintf(e1->components[0].name, sizeof(e1->components[0].name),
             "Position");
    snprintf(e1->components[0].jsonValue, sizeof(e1->components[0].jsonValue),
             "{\"x\": 45.0, \"y\": 0.0, \"z\": 10.0}");
    snprintf(e1->components[1].name, sizeof(e1->components[1].name),
             "Velocity");
    snprintf(e1->components[1].jsonValue, sizeof(e1->components[1].jsonValue),
             "{\"dx\": -0.5, \"dy\": 0.0, \"dz\": 0.2}");
    snprintf(e1->components[2].name, sizeof(e1->components[2].name), "Health");
    snprintf(e1->components[2].jsonValue, sizeof(e1->components[2].jsonValue),
             "{\"current\": 50, \"max\": 50}");
    snprintf(e1->components[3].name, sizeof(e1->components[3].name),
             "EnemyTag");
    snprintf(e1->components[3].jsonValue, sizeof(e1->components[3].jsonValue),
             "{}");

    // Entity 2: Enemy_02
    MockEntity *e2 = &outData->entities[2];
    e2->id = 0x1002;
    snprintf(e2->name, sizeof(e2->name), "Enemy_02");
    snprintf(e2->path, sizeof(e2->path), "game.actors.enemies.enemy_02");
    e2->componentCount = 2;
    snprintf(e2->components[0].name, sizeof(e2->components[0].name),
             "Position");
    snprintf(e2->components[0].jsonValue, sizeof(e2->components[0].jsonValue),
             "{\"x\": 62.0, \"y\": 0.0, \"z\": -15.0}");
    snprintf(e2->components[1].name, sizeof(e2->components[1].name), "Health");
    snprintf(e2->components[1].jsonValue, sizeof(e2->components[1].jsonValue),
             "{\"current\": 75, \"max\": 75}");

    // Entity 3: Camera_Main
    MockEntity *e3 = &outData->entities[3];
    e3->id = 0x1003;
    snprintf(e3->name, sizeof(e3->name), "Camera_Main");
    snprintf(e3->path, sizeof(e3->path), "game.rendering.camera_main");
    e3->componentCount = 2;
    snprintf(e3->components[0].name, sizeof(e3->components[0].name), "Camera");
    snprintf(e3->components[0].jsonValue, sizeof(e3->components[0].jsonValue),
             "{\"fov\": 60.0, \"near\": 0.1, \"far\": 1000.0}");
    snprintf(e3->components[1].name, sizeof(e3->components[1].name),
             "Transform");
    snprintf(e3->components[1].jsonValue, sizeof(e3->components[1].jsonValue),
             "{\"x\": 0.0, \"y\": 10.0, \"z\": -20.0}");

    // Entity 4: Light_Sun
    MockEntity *e4 = &outData->entities[4];
    e4->id = 0x1004;
    snprintf(e4->name, sizeof(e4->name), "Light_Sun");
    snprintf(e4->path, sizeof(e4->path), "game.rendering.light_sun");
    e4->componentCount = 1;
    snprintf(e4->components[0].name, sizeof(e4->components[0].name),
             "DirectionalLight");
    snprintf(e4->components[0].jsonValue, sizeof(e4->components[0].jsonValue),
             "{\"color\": \"#FFF8E7\", \"intensity\": 1.2}");

    // Entity 5: Bullet_001
    MockEntity *e5 = &outData->entities[5];
    e5->id = 0x1005;
    snprintf(e5->name, sizeof(e5->name), "Bullet_001");
    snprintf(e5->path, sizeof(e5->path), "game.projectiles.bullet_001");
    e5->componentCount = 3;
    snprintf(e5->components[0].name, sizeof(e5->components[0].name),
             "Position");
    snprintf(e5->components[0].jsonValue, sizeof(e5->components[0].jsonValue),
             "{\"x\": 15.0, \"y\": 1.2, \"z\": -4.0}");
    snprintf(e5->components[1].name, sizeof(e5->components[1].name),
             "Velocity");
    snprintf(e5->components[1].jsonValue, sizeof(e5->components[1].jsonValue),
             "{\"dx\": 25.0, \"dy\": 0.0, \"dz\": 0.0}");
    snprintf(e5->components[2].name, sizeof(e5->components[2].name), "Damage");
    snprintf(e5->components[2].jsonValue, sizeof(e5->components[2].jsonValue),
             "{\"amount\": 20}");

    // Entity 6: Platform_A
    MockEntity *e6 = &outData->entities[6];
    e6->id = 0x1006;
    snprintf(e6->name, sizeof(e6->name), "Platform_A");
    snprintf(e6->path, sizeof(e6->path), "game.world.platform_a");
    e6->componentCount = 2;
    snprintf(e6->components[0].name, sizeof(e6->components[0].name),
             "Transform");
    snprintf(e6->components[0].jsonValue, sizeof(e6->components[0].jsonValue),
             "{\"x\": 0.0, \"y\": -1.0, \"z\": 0.0}");
    snprintf(e6->components[1].name, sizeof(e6->components[1].name),
             "StaticMesh");
    snprintf(e6->components[1].jsonValue, sizeof(e6->components[1].jsonValue),
             "{\"mesh\": \"cube_1x1\", \"material\": \"stone\"}");
}

/**
 * Renders mock content inside the given panel window using Darcula styling.
 *
 * @param renderer Dedicated PanelRenderer instance.
 * @param win Curses WINDOW to render content into.
 * @param panel PanelId identifying which view content to draw.
 * @param data Mock world data snapshot.
 * @param isFocused true if this panel currently holds keyboard/mouse focus.
 */
void PanelMockRenderContent(PanelRenderer *renderer, WINDOW *win, PanelId panel,
                            const MockWorldData *data, bool isFocused)
{
    (void)isFocused;
    if (win == NULL || data == NULL) {
        return;
    }

    switch (panel) {
    case PANEL_HEADER: {
        char buf[256];
        snprintf(buf, sizeof(buf), "Flecs Explorer v0.1.0");
        PanelRendererDrawText(renderer, win, 1, 2, buf, PANEL_COLOR_CHROME);

        snprintf(buf, sizeof(buf), "World: %s", data->worldName);
        PanelRendererDrawText(renderer, win, 1, 28, buf, PANEL_COLOR_KEYWORD);

        snprintf(buf, sizeof(buf), "Status: %s", data->status);
        PanelRendererDrawText(renderer, win, 1, 55, buf,
                              PANEL_COLOR_STATUS_ONLINE);

        snprintf(buf, sizeof(buf),
                 "Entities: %d  |  Tables: %d  |  "
                 "Systems: %d  |  FPS: %.1f",
                 data->entityCount, data->tableCount, data->systemCount,
                 data->fps);
        PanelRendererDrawText(renderer, win, 2, 2, buf, PANEL_COLOR_DEFAULT);
        break;
    }

    case PANEL_ENTITIES: {
        PanelRendererDrawText(renderer, win, 1, 2,
                              "  #  NAME              COMPONENTS",
                              PANEL_COLOR_CHROME);

        for (int i = 0; i < data->totalEntities; ++i) {
            char buf[128];
            const bool isSelected = (i == data->selectedEntityIndex);

            if (isSelected) {
                snprintf(buf, sizeof(buf), "> [%d] %-16s (%d comps)", i,
                         data->entities[i].name,
                         data->entities[i].componentCount);
                PanelRendererDrawText(renderer, win, 2 + i, 2, buf,
                                      PANEL_COLOR_SELECTION);
            } else {
                snprintf(buf, sizeof(buf), "  [%d] %-16s (%d comps)", i,
                         data->entities[i].name,
                         data->entities[i].componentCount);
                PanelRendererDrawText(renderer, win, 2 + i, 2, buf,
                                      PANEL_COLOR_DEFAULT);
            }
        }
        break;
    }

    case PANEL_INSPECTOR: {
        if (data->selectedEntityIndex >= 0 &&
            data->selectedEntityIndex < data->totalEntities) {
            const MockEntity *ent = &data->entities[data->selectedEntityIndex];
            char buf[576];

            snprintf(buf, sizeof(buf), "Entity: %s  (ID: 0x%lx)", ent->name,
                     (unsigned long)ent->id);
            PanelRendererDrawText(renderer, win, 1, 2, buf, PANEL_COLOR_CHROME);

            snprintf(buf, sizeof(buf), "Path:   %s", ent->path);
            PanelRendererDrawText(renderer, win, 2, 2, buf,
                                  PANEL_COLOR_DEFAULT);

            PanelRendererDrawText(renderer, win, 3, 2,
                                  "----------------------------------------",
                                  PANEL_COLOR_BORDER_INACTIVE);

            int row = 4;
            for (int c = 0; c < ent->componentCount; ++c) {
                snprintf(buf, sizeof(buf), "%s:", ent->components[c].name);
                PanelRendererDrawText(renderer, win, row++, 2, buf,
                                      PANEL_COLOR_KEYWORD);

                snprintf(buf, sizeof(buf), "  %s",
                         ent->components[c].jsonValue);
                PanelRendererDrawText(renderer, win, row++, 2, buf,
                                      PANEL_COLOR_NUMERIC);
            }
        }
        break;
    }

    case PANEL_STATUS: {
        PanelRendererDrawText(
            renderer, win, 0, 1,
            "[Tab] Focus  [j/k/Up/Dn] Select  [Click] Focus/Select  "
            "[Wheel] Scroll  [q] Quit",
            PANEL_COLOR_CHROME);
        break;
    }

    case PANEL_COUNT:
    default:
        break;
    }
}

// Player Menu

#include "gui_board.h"

void draw_menu(SDL_Renderer* renderer, TTF_Font* font, struct Config* config) {  
    float text_width, text_height;
    
    SDL_FRect menu_box = {
        .x = config->window_width / 8  * 2,
        .y = config->window_height / 8 * 1,
        .w = config->window_width / 8 * 4,
        .h = config->window_height / 8 * 6
    };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 230);    
    SDL_RenderFillRect(renderer, &menu_box);

    SDL_Texture* player_1_text = create_text(renderer, font, "1 Player");
    SDL_GetTextureSize(player_1_text, &text_width, &text_height);
    SDL_FRect button_rect_1_player = {
        .x = config->window_width / 8 * 2.5f,
        .y = config->window_height / 8 * 1.5f,
        .w = config->window_height / 8 * 3,
        .h = config->window_height / 8 * 1.5f
    }; 
    SDL_FRect text_rect_1 = {
        .w = text_width,
        .h = text_height,
        .x = button_rect_1_player.x + (button_rect_1_player.w - text_width) / 2.0f,
        .y = button_rect_1_player.y + (button_rect_1_player.h - text_height) / 2.0f
    };
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &button_rect_1_player);
    SDL_RenderTexture(renderer, player_1_text, NULL, &text_rect_1);

    SDL_Texture* player_2_text = create_text(renderer, font, "2 Players");
    SDL_GetTextureSize(player_2_text, &text_width, &text_height);
    SDL_FRect button_rect_2_player = {
        .x = config->window_width / 8 * 2.5f,
        .y = config->window_height / 8 * 3.5f,
        .w = config->window_height / 8 * 3,
        .h = config->window_height / 8 * 1.5f
    };
    SDL_FRect text_rect_2 = {
        .w = text_width,
        .h = text_height,
        .x = button_rect_2_player.x + (button_rect_2_player.w - text_width) / 2.0f,
        .y = button_rect_2_player.y + (button_rect_2_player.h - text_height) / 2.0f
    };
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    SDL_RenderFillRect(renderer, &button_rect_2_player);
    SDL_RenderTexture(renderer, player_2_text, NULL, &text_rect_2);

    SDL_Texture* exit_text = create_text(renderer, font, "Exit");
    SDL_GetTextureSize(exit_text, &text_width, &text_height);
    SDL_FRect exit_rect = {
        .x = config->window_width / 8 * 2.5f,
        .y = config->window_height / 8 * 5.5f,
        .w = config->window_width / 8 * 3,
        .h = config->window_height / 8 * 1
    };
    SDL_FRect text_rect_exit = {
        .w = text_width,
        .h = text_height,
        .x = exit_rect.x + (exit_rect.w - text_width) / 2.0f,
        .y = exit_rect.y + (exit_rect.h - text_height) / 2.0f
    };
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &exit_rect);
    SDL_RenderTexture(renderer, exit_text, NULL, &text_rect_exit);
}

SDL_Texture* create_menu_texture(SDL_Renderer* renderer, TTF_Font* font, struct Config* config) {
    SDL_Texture* menu_texture = SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        config->window_width, config->window_height);

    SDL_SetRenderTarget(renderer, menu_texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0); // transparent background
    SDL_RenderClear(renderer);
    draw_menu(renderer, font, config);
    SDL_SetRenderTarget(renderer, NULL);
    return menu_texture;
}

SDL_Texture* create_text(SDL_Renderer* renderer, TTF_Font* font, const char* text) {
    SDL_Color color = {0, 0, 0, 0};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, strlen(text), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_DestroySurface(surface);
    return texture;
}

// HARD CODED! ToDo! Move to config file
TTF_Font* load_ttf_font(int font_size) {
    const char* font_path = "./resources/font/theboldfont.ttf";
    TTF_Font* font = TTF_OpenFont(font_path, font_size);

    if (!font) {
        printf("Font load error: %s\n", SDL_GetError());
    }
    return font;
}




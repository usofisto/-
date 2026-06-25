#include "gui.h"

void AppendUnicodeToUTF8(std::string& str, int codepoint) {
    if (codepoint <= 0x7F) {
        str += (char)codepoint;
    } else if (codepoint <= 0x7FF) {
        str += (char)(0xC0 | ((codepoint >> 6) & 0x1F));
        str += (char)(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        str += (char)(0xE0 | ((codepoint >> 12) & 0x0F));
        str += (char)(0x80 | ((codepoint >> 6) & 0x3F));
        str += (char)(0x80 | (codepoint & 0x3F));
    }
}

void AddLogMessage(const std::string& msg, std::vector<std::pair<std::string, float>>& log) {
    log.push_back({ msg, 4.0f });
    if (log.size() > 5) {
        log.erase(log.begin());
    }
}

void DrawWrappedText(Font font, const char* text, int posX, int posY, int maxWidth, int fontSize, Color color) {
    std::string textStr(text);
    std::vector<std::string> words;
    std::string currentWord = "";
    
    for (size_t i = 0; i < textStr.length(); ++i) {
        if (textStr[i] == ' ') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord = "";
            }
        } else if (textStr[i] == '\n') {
            if (!currentWord.empty()) {
                words.push_back(currentWord);
                currentWord = "";
            }
            words.push_back("\n");
        } else {
            currentWord += textStr[i];
        }
    }
    if (!currentWord.empty()) {
        words.push_back(currentWord);
    }
    
    std::string currentLine = "";
    int currentY = posY;
    
    for (const auto& word : words) {
        if (word == "\n") {
            DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
            currentY += fontSize + 4;
            currentLine = "";
            continue;
        }
        
        std::string testLine = currentLine + (currentLine.empty() ? "" : " ") + word;
        Vector2 size = MeasureTextEx(font, testLine.c_str(), (float)fontSize, 1.0f);
        if (size.x > maxWidth) {
            DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
            currentY += fontSize + 4;
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    
    if (!currentLine.empty()) {
        DrawTextEx(font, currentLine.c_str(), Vector2{ (float)posX, (float)currentY }, (float)fontSize, 1.0f, color);
    }
}

bool DrawButton(Font font, Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color clickColor, Color textColor) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rect);
    bool clicked = false;
    Color drawColor = baseColor;
    
    if (hovered) {
        drawColor = hoverColor;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            drawColor = clickColor;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            clicked = true;
        }
    }
    
    DrawRectangleRounded(rect, 0.25f, 4, drawColor);
    DrawRectangleRoundedLines(rect, 0.25f, 4, 1.5f, Color{ 255, 255, 255, 30 });
    
    float fontSize = 18.0f;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1.0f);
    DrawTextEx(font, text, Vector2{ rect.x + (rect.width - textSize.x)/2.0f, rect.y + (rect.height - textSize.y)/2.0f }, fontSize, 1.0f, textColor);
    
    return clicked;
}

void DrawProgressBar(Rectangle rect, float value, float maxValue, Color barColor, Color bgColor) {
    // Тень под баром
    DrawRectangleRounded(Rectangle{rect.x + 2, rect.y + 2, rect.width, rect.height}, 0.3f, 4, Color{0, 0, 0, 150});
    
    // Фон бара
    DrawRectangleRounded(rect, 0.3f, 4, bgColor);
    
    float fillWidth = (value / maxValue) * rect.width;
    if (fillWidth < 0.0f) fillWidth = 0.0f;
    if (fillWidth > rect.width) fillWidth = rect.width;
    
    if (fillWidth > 0.0f) {
        Rectangle fillRect = { rect.x, rect.y, fillWidth, rect.height };
        // Темный низ, светлый верх для объема
        Color lightColor = ColorTint(barColor, Color{255, 255, 255, 255});
        lightColor.r = std::min(255, lightColor.r + 40);
        lightColor.g = std::min(255, lightColor.g + 40);
        lightColor.b = std::min(255, lightColor.b + 40);
        
        Color darkColor = ColorTint(barColor, Color{150, 150, 150, 255});
        
        // Отрисовка градиента вручную для rounded rectangle (эмуляция: рисуем сплошной, потом градиент поверх)
        DrawRectangleRounded(fillRect, 0.3f, 4, barColor);
        
        // Внутренний блик сверху
        Rectangle highlightRect = { rect.x + 1, rect.y + 1, fillWidth - 2, rect.height * 0.3f };
        DrawRectangleRounded(highlightRect, 0.3f, 4, Color{255, 255, 255, 60});
    }
    
    // Рамка (Border)
    DrawRectangleRoundedLines(rect, 0.3f, 4, 2.0f, Color{ 30, 30, 30, 255 });
    DrawRectangleRoundedLines(Rectangle{rect.x-1, rect.y-1, rect.width+2, rect.height+2}, 0.3f, 4, 1.0f, Color{ 80, 80, 80, 100 });
}

bool DrawClassCard(Font font, Rectangle rect, const char* title, const char* stats, const char* desc, bool selected, Color cardBgColor, Color borderColor) {
    Vector2 mousePos = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mousePos, rect);
    bool clicked = false;
    
    if (hovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        clicked = true;
    }
    
    Color bg = cardBgColor;
    Color border = borderColor;
    if (hovered) {
        bg = Color{ 45, 50, 62, 255 };
        border = Color{ 99, 102, 241, 200 };
    }
    if (selected) {
        bg = Color{ 50, 56, 70, 255 };
        border = Color{ 99, 102, 241, 255 };
    }
    
    DrawRectangleRounded(rect, 0.1f, 4, bg);
    DrawRectangleRoundedLines(rect, 0.1f, 4, 2.0f, border);
    
    DrawTextEx(font, title, Vector2{ rect.x + 20, rect.y + 20 }, 22.0f, 1.0f, Color{ 243, 244, 246, 255 });
    DrawTextEx(font, stats, Vector2{ rect.x + 20, rect.y + 55 }, 16.0f, 1.0f, Color{ 245, 158, 11, 255 });
    DrawWrappedText(font, desc, rect.x + 20, rect.y + 90, rect.width - 40, 14, Color{ 156, 163, 175, 255 });
    
    return clicked;
}

#pragma once
#include "raylib.h"
#include <string>
#include <vector>

// Кодирование юникод-кодов в UTF-8
void AppendUnicodeToUTF8(std::string& str, int codepoint);

// Добавление сообщения в лог
void AddLogMessage(const std::string& msg, std::vector<std::pair<std::string, float>>& log);

// Отрисовка текста с переносом строк
void DrawWrappedText(Font font, const char* text, int posX, int posY, int maxWidth, int fontSize, Color color);

// Отрисовка кнопки
bool DrawButton(Font font, Rectangle rect, const char* text, Color baseColor, Color hoverColor, Color clickColor, Color textColor);

// Отрисовка прогресс-бара
void DrawProgressBar(Rectangle rect, float value, float maxValue, Color barColor, Color bgColor);

// Отрисовка карточки класса
bool DrawClassCard(Font font, Rectangle rect, const char* title, const char* stats, const char* desc, bool selected, Color cardBgColor, Color borderColor);

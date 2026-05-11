#include "TextRenderer.hpp"
#include "Window.hpp"

#include <cctype>


void TextRenderer::drawText(
    gfx::Window& win, 
    const std::string& text, 
    int x, int y, 
    int scale, 
    const gfx::Color& color
) 
{
    int cursorX = x;
    
    // Notre dictionnaire de police pixelisée 3x5 ! 
    // Chaque lettre est représentée par 5 entiers (5 lignes de 3 bits)
    for (char c : text) 
    {
        if (c == ' ') 
        {
            cursorX += 4 * scale;
            continue;
        }

        int data[5] = {0};

        switch (std::toupper(c)) 
        {
            // Alphabet complet (police 3x5 pixels)
            case 'A': data[0]=2; data[1]=5; data[2]=7; data[3]=5; data[4]=5; break;
            case 'B': data[0]=6; data[1]=5; data[2]=6; data[3]=5; data[4]=6; break;
            case 'C': data[0]=3; data[1]=4; data[2]=4; data[3]=4; data[4]=3; break;
            case 'D': data[0]=6; data[1]=5; data[2]=5; data[3]=5; data[4]=6; break;
            case 'E': data[0]=7; data[1]=4; data[2]=6; data[3]=4; data[4]=7; break;
            case 'F': data[0]=7; data[1]=4; data[2]=6; data[3]=4; data[4]=4; break;
            case 'G': data[0]=3; data[1]=4; data[2]=5; data[3]=5; data[4]=3; break;
            case 'H': data[0]=5; data[1]=5; data[2]=7; data[3]=5; data[4]=5; break;
            case 'I': data[0]=7; data[1]=2; data[2]=2; data[3]=2; data[4]=7; break;
            case 'J': data[0]=1; data[1]=1; data[2]=1; data[3]=5; data[4]=2; break;
            case 'K': data[0]=5; data[1]=6; data[2]=4; data[3]=6; data[4]=5; break;
            case 'L': data[0]=4; data[1]=4; data[2]=4; data[3]=4; data[4]=7; break;
            case 'M': data[0]=5; data[1]=7; data[2]=5; data[3]=5; data[4]=5; break;
            case 'N': data[0]=5; data[1]=7; data[2]=7; data[3]=5; data[4]=5; break;
            case 'O': data[0]=2; data[1]=5; data[2]=5; data[3]=5; data[4]=2; break;
            case 'P': data[0]=7; data[1]=5; data[2]=7; data[3]=4; data[4]=4; break;
            case 'Q': data[0]=2; data[1]=5; data[2]=5; data[3]=6; data[4]=3; break;
            case 'R': data[0]=7; data[1]=5; data[2]=7; data[3]=6; data[4]=5; break;
            case 'S': data[0]=3; data[1]=4; data[2]=2; data[3]=1; data[4]=6; break;
            case 'T': data[0]=7; data[1]=2; data[2]=2; data[3]=2; data[4]=2; break;
            case 'U': data[0]=5; data[1]=5; data[2]=5; data[3]=5; data[4]=7; break;
            case 'V': data[0]=5; data[1]=5; data[2]=5; data[3]=2; data[4]=2; break;
            case 'W': data[0]=5; data[1]=5; data[2]=5; data[3]=7; data[4]=5; break;
            case 'X': data[0]=5; data[1]=5; data[2]=2; data[3]=5; data[4]=5; break;
            case 'Y': data[0]=5; data[1]=5; data[2]=2; data[3]=2; data[4]=2; break;
            case 'Z': data[0]=7; data[1]=1; data[2]=2; data[3]=4; data[4]=7; break;
            // Chiffres
            case '0': data[0]=7; data[1]=5; data[2]=5; data[3]=5; data[4]=7; break;
            case '1': data[0]=2; data[1]=6; data[2]=2; data[3]=2; data[4]=7; break;
            case '2': data[0]=6; data[1]=1; data[2]=2; data[3]=4; data[4]=7; break;
            case '3': data[0]=6; data[1]=1; data[2]=2; data[3]=1; data[4]=6; break;
            case '4': data[0]=5; data[1]=5; data[2]=7; data[3]=1; data[4]=1; break;
            case '5': data[0]=7; data[1]=4; data[2]=6; data[3]=1; data[4]=6; break;
            case '6': data[0]=3; data[1]=4; data[2]=7; data[3]=5; data[4]=7; break;
            case '7': data[0]=7; data[1]=1; data[2]=2; data[3]=2; data[4]=2; break;
            case '8': data[0]=7; data[1]=5; data[2]=7; data[3]=5; data[4]=7; break;
            case '9': data[0]=7; data[1]=5; data[2]=7; data[3]=1; data[4]=6; break;
            // Symboles
            case '[': data[0]=6; data[1]=4; data[2]=4; data[3]=4; data[4]=6; break; 
            case ']': data[0]=3; data[1]=1; data[2]=1; data[3]=1; data[4]=3; break; 
            case '<': data[0]=1; data[1]=2; data[2]=4; data[3]=2; data[4]=1; break;
            case '>': data[0]=4; data[1]=2; data[2]=1; data[3]=2; data[4]=4; break;
            case '?': data[0]=6; data[1]=1; data[2]=2; data[3]=0; data[4]=2; break;
            case '!': data[0]=2; data[1]=2; data[2]=2; data[3]=0; data[4]=2; break;
            case ':': data[0]=0; data[1]=2; data[2]=0; data[3]=2; data[4]=0; break;
        }

        // Dessin des pixels de la lettre
        for (int row = 0; row < 5; ++row) 
        {
            int rowData = data[row];
            
            for (int col = 0; col < 3; ++col) 
            {
                // Lecture du bit correspondant
                if ((rowData >> (2 - col)) & 1) 
                {
                    win.fillRect(
                        cursorX + col * scale, 
                        y + row * scale, 
                        scale, 
                        scale, 
                        color
                    );
                }
            }
        }
        
        cursorX += 4 * scale; // Espacement avec la prochaine lettre
    }
}

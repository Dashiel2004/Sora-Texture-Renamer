#include <windows.h>
#include <commctrl.h>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace std;
using namespace fs;



vector<wstring> kh2Forms = {
    L"default",
    L"valor",
    L"limit",
    L"wisdom",
    L"master",
    L"final"
};

vector<wstring> kh3Forms = {
    L"default",
    L"second",
    L"guardian",
    L"strike",
    L"element",
    L"blitz",
    L"ultimate",
    L"light",
    L"dark"
};
const vector<wstring> slots = {
    L"c01",
    L"c03",
    L"c05",
    L"c07"
};
bool isKH3Slot(const wstring& slot)
{
    return (slot == L"c03" || slot == L"c07");
}
HWND dropdowns[5];
HWND slotDropdown;

// Helper to convert std::string to std::wstring
wstring to_wstring(const string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

bool generateOutfit(
    const wstring& form,
    int outfitSlot,
    const wstring& slot,
    const wstring& assetFolder,
    vector<wstring>& missingFiles)
{

    // Choose base name prefix depending on whether the selected slot is a KH3 slot.
    bool kh3 = isKH3Slot(slot);
    wstring baseName = (kh3 ? L"def_trail3_00" : L"def_trail2_00") + to_wstring(outfitSlot);

    vector<wstring> types = { L"col", L"nor", L"prm" };

    for (auto& type : types)
    {
        path source = L"assets/" + assetFolder + L"/" + form + L"_" + type + L".nutexb";
        path dest = L"output/" + slot + L"/" + baseName + L"_" + type + L".nutexb";

        if (!exists(source))
        {
            missingFiles.push_back(source.wstring());
            continue;
        }

        create_directories(L"output/" + slot);

        copy_file(source, dest, fs::copy_options::overwrite_existing);
    }

    return true;
}

void generateTextures()
{
    vector<wstring> missingFiles;

    int slotSelection = SendMessage(slotDropdown, CB_GETCURSEL, 0, 0);

    if (slotSelection < 0 || slotSelection >= slots.size())
        return;

    wstring selectedSlot = slots[slotSelection];

    bool kh3 = isKH3Slot(selectedSlot);

    vector<wstring>& forms = kh3 ? kh3Forms : kh2Forms;

    wstring assetFolder = kh3 ? L"kh3" : L"kh2";

    for (int i = 0; i < 5; i++)
    {
        int selection = SendMessage(dropdowns[i], CB_GETCURSEL, 0, 0);

        if (selection >= 0 && selection < forms.size())
        {
            generateOutfit(forms[selection], i + 1, selectedSlot, assetFolder, missingFiles);
        }
    }

    if (!missingFiles.empty())
    {
        wstring errorMsg = L"The following textures are missing:\n\n";

        for (auto& file : missingFiles)
            errorMsg += file + L"\n";

        MessageBox(NULL, errorMsg.c_str(), L"Missing Textures", MB_ICONERROR);
        return;
    }

    MessageBox(NULL, L"Textures generated successfully!", L"Success", MB_OK);
}
void updateFormDropdowns(bool kh3)
{
    vector<wstring>& forms = kh3 ? kh3Forms : kh2Forms;

    for (int i = 0; i < 5; i++)
    {
        SendMessage(dropdowns[i], CB_RESETCONTENT, 0, 0);

        for (auto& form : forms)
        {
            SendMessage(dropdowns[i], CB_ADDSTRING, 0, (LPARAM)form.c_str());
        }

        SendMessage(dropdowns[i], CB_SETCURSEL, 0, 0);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        CreateWindow(
            L"STATIC",
            L"Slot:",
            WS_VISIBLE | WS_CHILD,
            20,
            20,
            60,
            20,
            hwnd,
            NULL,
            NULL,
            NULL
        );

        slotDropdown = CreateWindow(
            L"COMBOBOX",
            L"",
            CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
            100,
            20,
            150,
            200,
            hwnd,
            NULL,
            NULL,
            NULL
        );

        for (auto& slot : slots)
        {
            SendMessage(slotDropdown, CB_ADDSTRING, 0, (LPARAM)slot.c_str());
        }
        for (int i = 0; i < 5; i++)
        {
            SendMessage(slotDropdown, CB_SETCURSEL, 0, 0);
            wstring label = L"Outfit " + to_wstring(i + 1) + L":";
            CreateWindowW(
                L"STATIC",
                label.c_str(),
                WS_VISIBLE | WS_CHILD,
                20,
                60 + i * 40,
                70,
                20,
                hwnd,
                NULL,
                NULL,
                NULL
            );

            dropdowns[i] = CreateWindowW(
                L"COMBOBOX",
                L"",
                CBS_DROPDOWNLIST | WS_VISIBLE | WS_CHILD,
                100,
                60 + i * 40,
                150,
                200,
                hwnd,
                NULL,
                NULL,
                NULL
            );

            vector<wstring>& forms = kh2Forms;

            for (auto& form : forms)
            {
                SendMessage(dropdowns[i], CB_ADDSTRING, 0, (LPARAM)form.c_str());
            }

            SendMessage(dropdowns[i], CB_SETCURSEL, 0, 0);
        }

        CreateWindowW(
            L"BUTTON",
            L"Rename Textures",
            WS_VISIBLE | WS_CHILD,
            70,
            265,
            150,
            30,
            hwnd,
            (HMENU)1,
            NULL,
            NULL
        );
    }
    break;

    case WM_COMMAND:
        if ((HWND)lParam == slotDropdown && HIWORD(wParam) == CBN_SELCHANGE)
        {
            int selection = SendMessage(slotDropdown, CB_GETCURSEL, 0, 0);

            if (selection >= 0)
            {
                bool kh3 = isKH3Slot(slots[selection]);
                updateFormDropdowns(kh3);
            }
        }
        if (LOWORD(wParam) == 1)
        {
            generateTextures();
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"SoraTextureTool";

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(131));
    wc.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(131));

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"Rename Sora Textures",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        300, 350,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
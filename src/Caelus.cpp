#include "CaelusElement.h"
#include "CaelusWindow.h"

#include "Caelus.h"

namespace Caelus
{
    void Init(HINSTANCE hInstance)
    {
        CaelusWindow::Register(hInstance);
    }
}
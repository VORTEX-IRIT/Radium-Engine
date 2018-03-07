#ifndef RADIUMENGINE_KEYEVENT_HPP
#define RADIUMENGINE_KEYEVENT_HPP

#include <Core/Event/EventEnums.hpp>
#include <Core/RaCore.hpp>

namespace Ra {
namespace Core {

struct KeyEvent {
    /// KeyEventType : Press, Release
    int event;
    /// Keycode. Test this with Qt::Keys enum values.
    int key;

    /// Modifier used : Ctrl, Alt, Shift.
    int modifier;
};

} // namespace Core
} // namespace Ra

#endif // RADIUMENGINE_KEYEVENT_HPP

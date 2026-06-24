/****************************************************************************
** Meta object code from reading C++ file 'sdlgamepadkeynavigation.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../gui/sdlgamepadkeynavigation.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sdlgamepadkeynavigation.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SdlGamepadKeyNavigation_t {
    QByteArrayData data[10];
    char stringdata0[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SdlGamepadKeyNavigation_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SdlGamepadKeyNavigation_t qt_meta_stringdata_SdlGamepadKeyNavigation = {
    {
QT_MOC_LITERAL(0, 0, 23), // "SdlGamepadKeyNavigation"
QT_MOC_LITERAL(1, 24, 19), // "onPollingTimerFired"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 6), // "enable"
QT_MOC_LITERAL(4, 52, 7), // "disable"
QT_MOC_LITERAL(5, 60, 17), // "notifyWindowFocus"
QT_MOC_LITERAL(6, 78, 8), // "hasFocus"
QT_MOC_LITERAL(7, 87, 12), // "setUiNavMode"
QT_MOC_LITERAL(8, 100, 12), // "settingsMode"
QT_MOC_LITERAL(9, 113, 20) // "getConnectedGamepads"

    },
    "SdlGamepadKeyNavigation\0onPollingTimerFired\0"
    "\0enable\0disable\0notifyWindowFocus\0"
    "hasFocus\0setUiNavMode\0settingsMode\0"
    "getConnectedGamepads"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SdlGamepadKeyNavigation[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
       3,    0,   45,    2, 0x02 /* Public */,
       4,    0,   46,    2, 0x02 /* Public */,
       5,    1,   47,    2, 0x02 /* Public */,
       7,    1,   50,    2, 0x02 /* Public */,
       9,    0,   53,    2, 0x02 /* Public */,

 // slots: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    6,
    QMetaType::Void, QMetaType::Bool,    8,
    QMetaType::Int,

       0        // eod
};

void SdlGamepadKeyNavigation::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SdlGamepadKeyNavigation *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onPollingTimerFired(); break;
        case 1: _t->enable(); break;
        case 2: _t->disable(); break;
        case 3: _t->notifyWindowFocus((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 4: _t->setUiNavMode((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: { int _r = _t->getConnectedGamepads();
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SdlGamepadKeyNavigation::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_SdlGamepadKeyNavigation.data,
    qt_meta_data_SdlGamepadKeyNavigation,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SdlGamepadKeyNavigation::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SdlGamepadKeyNavigation::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SdlGamepadKeyNavigation.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SdlGamepadKeyNavigation::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'systemproperties.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../backend/systemproperties.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'systemproperties.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SystemProperties_t {
    QByteArrayData data[29];
    char stringdata0[541];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SystemProperties_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SystemProperties_t qt_meta_stringdata_SystemProperties = {
    {
QT_MOC_LITERAL(0, 0, 16), // "SystemProperties"
QT_MOC_LITERAL(1, 17, 23), // "unmappedGamepadsChanged"
QT_MOC_LITERAL(2, 41, 0), // ""
QT_MOC_LITERAL(3, 42, 30), // "hasHardwareAccelerationChanged"
QT_MOC_LITERAL(4, 73, 31), // "rendererAlwaysFullScreenChanged"
QT_MOC_LITERAL(5, 105, 24), // "maximumResolutionChanged"
QT_MOC_LITERAL(6, 130, 18), // "supportsHdrChanged"
QT_MOC_LITERAL(7, 149, 23), // "updateDecoderProperties"
QT_MOC_LITERAL(8, 173, 23), // "hasHardwareAcceleration"
QT_MOC_LITERAL(9, 197, 24), // "rendererAlwaysFullScreen"
QT_MOC_LITERAL(10, 222, 17), // "maximumResolution"
QT_MOC_LITERAL(11, 240, 11), // "supportsHdr"
QT_MOC_LITERAL(12, 252, 19), // "getNativeResolution"
QT_MOC_LITERAL(13, 272, 12), // "displayIndex"
QT_MOC_LITERAL(14, 285, 21), // "getSafeAreaResolution"
QT_MOC_LITERAL(15, 307, 14), // "getRefreshRate"
QT_MOC_LITERAL(16, 322, 14), // "startAsyncLoad"
QT_MOC_LITERAL(17, 337, 16), // "waitForAsyncLoad"
QT_MOC_LITERAL(18, 354, 15), // "refreshDisplays"
QT_MOC_LITERAL(19, 370, 16), // "isRunningWayland"
QT_MOC_LITERAL(20, 387, 17), // "isRunningXWayland"
QT_MOC_LITERAL(21, 405, 7), // "isWow64"
QT_MOC_LITERAL(22, 413, 22), // "friendlyNativeArchName"
QT_MOC_LITERAL(23, 436, 21), // "hasDesktopEnvironment"
QT_MOC_LITERAL(24, 458, 10), // "hasBrowser"
QT_MOC_LITERAL(25, 469, 21), // "hasDiscordIntegration"
QT_MOC_LITERAL(26, 491, 18), // "usesMaterial3Theme"
QT_MOC_LITERAL(27, 510, 13), // "versionString"
QT_MOC_LITERAL(28, 524, 16) // "unmappedGamepads"

    },
    "SystemProperties\0unmappedGamepadsChanged\0"
    "\0hasHardwareAccelerationChanged\0"
    "rendererAlwaysFullScreenChanged\0"
    "maximumResolutionChanged\0supportsHdrChanged\0"
    "updateDecoderProperties\0hasHardwareAcceleration\0"
    "rendererAlwaysFullScreen\0maximumResolution\0"
    "supportsHdr\0getNativeResolution\0"
    "displayIndex\0getSafeAreaResolution\0"
    "getRefreshRate\0startAsyncLoad\0"
    "waitForAsyncLoad\0refreshDisplays\0"
    "isRunningWayland\0isRunningXWayland\0"
    "isWow64\0friendlyNativeArchName\0"
    "hasDesktopEnvironment\0hasBrowser\0"
    "hasDiscordIntegration\0usesMaterial3Theme\0"
    "versionString\0unmappedGamepads"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SystemProperties[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
      14,  100, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   74,    2, 0x06 /* Public */,
       3,    0,   75,    2, 0x06 /* Public */,
       4,    0,   76,    2, 0x06 /* Public */,
       5,    0,   77,    2, 0x06 /* Public */,
       6,    0,   78,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    4,   79,    2, 0x08 /* Private */,

 // methods: name, argc, parameters, tag, flags
      12,    1,   88,    2, 0x02 /* Public */,
      14,    1,   91,    2, 0x02 /* Public */,
      15,    1,   94,    2, 0x02 /* Public */,
      16,    0,   97,    2, 0x02 /* Public */,
      17,    0,   98,    2, 0x02 /* Public */,
      18,    0,   99,    2, 0x02 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool, QMetaType::Bool, QMetaType::QSize, QMetaType::Bool,    8,    9,   10,   11,

 // methods: parameters
    QMetaType::QRect, QMetaType::Int,   13,
    QMetaType::QRect, QMetaType::Int,   13,
    QMetaType::Int, QMetaType::Int,   13,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      19, QMetaType::Bool, 0x00095401,
      20, QMetaType::Bool, 0x00095401,
      21, QMetaType::Bool, 0x00095401,
      22, QMetaType::QString, 0x00095401,
      23, QMetaType::Bool, 0x00095401,
      24, QMetaType::Bool, 0x00095401,
      25, QMetaType::Bool, 0x00095401,
      26, QMetaType::Bool, 0x00095401,
      27, QMetaType::QString, 0x00095401,
       8, QMetaType::Bool, 0x00495003,
       9, QMetaType::Bool, 0x00495003,
      28, QMetaType::QString, 0x00495003,
      10, QMetaType::QSize, 0x00495003,
      11, QMetaType::Bool, 0x00495003,

 // properties: notify_signal_id
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       1,
       2,
       0,
       3,
       4,

       0        // eod
};

void SystemProperties::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SystemProperties *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->unmappedGamepadsChanged(); break;
        case 1: _t->hasHardwareAccelerationChanged(); break;
        case 2: _t->rendererAlwaysFullScreenChanged(); break;
        case 3: _t->maximumResolutionChanged(); break;
        case 4: _t->supportsHdrChanged(); break;
        case 5: _t->updateDecoderProperties((*reinterpret_cast< bool(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< QSize(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 6: { QRect _r = _t->getNativeResolution((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QRect*>(_a[0]) = std::move(_r); }  break;
        case 7: { QRect _r = _t->getSafeAreaResolution((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QRect*>(_a[0]) = std::move(_r); }  break;
        case 8: { int _r = _t->getRefreshRate((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< int*>(_a[0]) = std::move(_r); }  break;
        case 9: _t->startAsyncLoad(); break;
        case 10: _t->waitForAsyncLoad(); break;
        case 11: _t->refreshDisplays(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SystemProperties::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SystemProperties::unmappedGamepadsChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SystemProperties::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SystemProperties::hasHardwareAccelerationChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (SystemProperties::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SystemProperties::rendererAlwaysFullScreenChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (SystemProperties::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SystemProperties::maximumResolutionChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (SystemProperties::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SystemProperties::supportsHdrChanged)) {
                *result = 4;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<SystemProperties *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->isRunningWayland; break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->isRunningXWayland; break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->isWow64; break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->friendlyNativeArchName; break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->hasDesktopEnvironment; break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->hasBrowser; break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->hasDiscordIntegration; break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->usesMaterial3Theme; break;
        case 8: *reinterpret_cast< QString*>(_v) = _t->versionString; break;
        case 9: *reinterpret_cast< bool*>(_v) = _t->hasHardwareAcceleration; break;
        case 10: *reinterpret_cast< bool*>(_v) = _t->rendererAlwaysFullScreen; break;
        case 11: *reinterpret_cast< QString*>(_v) = _t->unmappedGamepads; break;
        case 12: *reinterpret_cast< QSize*>(_v) = _t->maximumResolution; break;
        case 13: *reinterpret_cast< bool*>(_v) = _t->supportsHdr; break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<SystemProperties *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 9:
            if (_t->hasHardwareAcceleration != *reinterpret_cast< bool*>(_v)) {
                _t->hasHardwareAcceleration = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->hasHardwareAccelerationChanged();
            }
            break;
        case 10:
            if (_t->rendererAlwaysFullScreen != *reinterpret_cast< bool*>(_v)) {
                _t->rendererAlwaysFullScreen = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->rendererAlwaysFullScreenChanged();
            }
            break;
        case 11:
            if (_t->unmappedGamepads != *reinterpret_cast< QString*>(_v)) {
                _t->unmappedGamepads = *reinterpret_cast< QString*>(_v);
                Q_EMIT _t->unmappedGamepadsChanged();
            }
            break;
        case 12:
            if (_t->maximumResolution != *reinterpret_cast< QSize*>(_v)) {
                _t->maximumResolution = *reinterpret_cast< QSize*>(_v);
                Q_EMIT _t->maximumResolutionChanged();
            }
            break;
        case 13:
            if (_t->supportsHdr != *reinterpret_cast< bool*>(_v)) {
                _t->supportsHdr = *reinterpret_cast< bool*>(_v);
                Q_EMIT _t->supportsHdrChanged();
            }
            break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject SystemProperties::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_SystemProperties.data,
    qt_meta_data_SystemProperties,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SystemProperties::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SystemProperties::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SystemProperties.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SystemProperties::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 12;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 14;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 14;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void SystemProperties::unmappedGamepadsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SystemProperties::hasHardwareAccelerationChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SystemProperties::rendererAlwaysFullScreenChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SystemProperties::maximumResolutionChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SystemProperties::supportsHdrChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'computerseeker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.18)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../backend/computerseeker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'computerseeker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.18. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ComputerSeeker_t {
    QByteArrayData data[8];
    char stringdata0[92];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ComputerSeeker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ComputerSeeker_t qt_meta_stringdata_ComputerSeeker = {
    {
QT_MOC_LITERAL(0, 0, 14), // "ComputerSeeker"
QT_MOC_LITERAL(1, 15, 13), // "computerFound"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 11), // "NvComputer*"
QT_MOC_LITERAL(4, 42, 8), // "computer"
QT_MOC_LITERAL(5, 51, 12), // "errorTimeout"
QT_MOC_LITERAL(6, 64, 17), // "onComputerUpdated"
QT_MOC_LITERAL(7, 82, 9) // "onTimeout"

    },
    "ComputerSeeker\0computerFound\0\0NvComputer*\0"
    "computer\0errorTimeout\0onComputerUpdated\0"
    "onTimeout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ComputerSeeker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   34,    2, 0x06 /* Public */,
       5,    0,   37,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       6,    1,   38,    2, 0x08 /* Private */,
       7,    0,   41,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

void ComputerSeeker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ComputerSeeker *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->computerFound((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 1: _t->errorTimeout(); break;
        case 2: _t->onComputerUpdated((*reinterpret_cast< NvComputer*(*)>(_a[1]))); break;
        case 3: _t->onTimeout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ComputerSeeker::*)(NvComputer * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerSeeker::computerFound)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ComputerSeeker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ComputerSeeker::errorTimeout)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ComputerSeeker::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_ComputerSeeker.data,
    qt_meta_data_ComputerSeeker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ComputerSeeker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ComputerSeeker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ComputerSeeker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ComputerSeeker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void ComputerSeeker::computerFound(NvComputer * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ComputerSeeker::errorTimeout()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

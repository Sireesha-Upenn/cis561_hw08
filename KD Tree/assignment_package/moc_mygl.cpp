/****************************************************************************
** Meta object code from reading C++ file 'mygl.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.14.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "src/ignore_me/mygl.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mygl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.14.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyGL_t {
    QByteArrayData data[14];
    char stringdata0[225];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyGL_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyGL_t qt_meta_stringdata_MyGL = {
    {
QT_MOC_LITERAL(0, 0, 4), // "MyGL"
QT_MOC_LITERAL(1, 5, 20), // "sig_updateNumSamples"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 20), // "sig_fixSampleModeBox"
QT_MOC_LITERAL(4, 48, 15), // "slot_setSamples"
QT_MOC_LITERAL(5, 64, 18), // "slot_setSampleMode"
QT_MOC_LITERAL(6, 83, 10), // "SampleMode"
QT_MOC_LITERAL(7, 94, 16), // "slot_buildKDTree"
QT_MOC_LITERAL(8, 111, 18), // "slot_updateSphereX"
QT_MOC_LITERAL(9, 130, 18), // "slot_updateSphereY"
QT_MOC_LITERAL(10, 149, 18), // "slot_updateSphereZ"
QT_MOC_LITERAL(11, 168, 18), // "slot_updateSphereR"
QT_MOC_LITERAL(12, 187, 19), // "slot_setDisplayGrid"
QT_MOC_LITERAL(13, 207, 17) // "slot_searchPoints"

    },
    "MyGL\0sig_updateNumSamples\0\0"
    "sig_fixSampleModeBox\0slot_setSamples\0"
    "slot_setSampleMode\0SampleMode\0"
    "slot_buildKDTree\0slot_updateSphereX\0"
    "slot_updateSphereY\0slot_updateSphereZ\0"
    "slot_updateSphereR\0slot_setDisplayGrid\0"
    "slot_searchPoints"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyGL[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       3,    1,   72,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   75,    2, 0x0a /* Public */,
       5,    1,   78,    2, 0x0a /* Public */,
       7,    0,   81,    2, 0x0a /* Public */,
       8,    1,   82,    2, 0x0a /* Public */,
       9,    1,   85,    2, 0x0a /* Public */,
      10,    1,   88,    2, 0x0a /* Public */,
      11,    1,   91,    2, 0x0a /* Public */,
      12,    0,   94,    2, 0x0a /* Public */,
      13,    0,   95,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, QMetaType::Int,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    2,
    QMetaType::Void, 0x80000000 | 6,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void, QMetaType::Double,    2,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void MyGL::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyGL *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sig_updateNumSamples((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->sig_fixSampleModeBox((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->slot_setSamples((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->slot_setSampleMode((*reinterpret_cast< SampleMode(*)>(_a[1]))); break;
        case 4: _t->slot_buildKDTree(); break;
        case 5: _t->slot_updateSphereX((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 6: _t->slot_updateSphereY((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 7: _t->slot_updateSphereZ((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 8: _t->slot_updateSphereR((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 9: _t->slot_setDisplayGrid(); break;
        case 10: _t->slot_searchPoints(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyGL::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGL::sig_updateNumSamples)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MyGL::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyGL::sig_fixSampleModeBox)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyGL::staticMetaObject = { {
    QMetaObject::SuperData::link<GLWidget277::staticMetaObject>(),
    qt_meta_stringdata_MyGL.data,
    qt_meta_data_MyGL,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyGL::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyGL::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyGL.stringdata0))
        return static_cast<void*>(this);
    return GLWidget277::qt_metacast(_clname);
}

int MyGL::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = GLWidget277::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void MyGL::sig_updateNumSamples(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void MyGL::sig_fixSampleModeBox(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

/* Copyright 2013-2018 Tim Greve
 *
 * ledpixelmath is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * ledpixelmath is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with ledpixelmath.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#include <Python.h>
#include <array>
#include <atomic>

typedef struct
{
    PyObject_HEAD
    std::atomic<uint8_t> r_akt{0};
    std::atomic<uint8_t> g_akt{0};
    std::atomic<uint8_t> b_akt{0};
    std::atomic<uint8_t> r_fadeTo{0};
    std::atomic<uint8_t> g_fadeTo{0};
    std::atomic<uint8_t> b_fadeTo{0};
    std::atomic_bool r_fadeDirection{false};
    std::atomic_bool g_fadeDirection{false};
    std::atomic_bool b_fadeDirection{false};
    std::atomic_bool fadeComplete{false};
    std::atomic<uint32_t> pixelIndex{0};
} PixelObject;

static void Pixel_dealloc(PixelObject* self);
static int Pixel_init(PixelObject* self, PyObject* arg);
static PyObject* Pixel_new(PyTypeObject* type, PyObject* arg, PyObject* kw);

static PyObject* Pixel_getFadeComplete(PixelObject* self);
static PyObject* Pixel_getIndex(PixelObject* self);
static PyObject* Pixel_trigger(PixelObject* self);
static PyObject* Pixel_fadeToRgb(PixelObject* self, PyObject* arg);
static PyObject* Pixel_fillRgb(PixelObject* self, PyObject* arg);

static PyMethodDef PixelMethods[] = {
        { "getFadeComplete", (PyCFunction)Pixel_getFadeComplete, METH_NOARGS, nullptr },
        { "getIndex", (PyCFunction)Pixel_getIndex, METH_NOARGS, nullptr },
        { "trigger", (PyCFunction)Pixel_trigger, METH_NOARGS, nullptr },
        { "fadeToRgb", (PyCFunction)Pixel_fadeToRgb, METH_VARARGS, nullptr },
        { "fillRgb", (PyCFunction)Pixel_fillRgb, METH_VARARGS, nullptr },
        { nullptr, nullptr, 0, nullptr }
};

static PyTypeObject PixelObjectType = {
        PyVarObject_HEAD_INIT(nullptr, 0)
        "ledpixelmath.Pixel",  // tp_name (module name, object name)
        sizeof(PixelObject),       // tp_basicsize
        0,                           // tp_itemsize
        (destructor)Pixel_dealloc, // tp_dealloc
        nullptr,                           // tp_print
        nullptr,                           // tp_getattr
        nullptr,                           // tp_setattr
        nullptr,                           // tp_as_async
        nullptr,                           // tp_repr
        nullptr,                           // tp_as_number
        nullptr,                           // tp_as_sequence
        nullptr,                           // tp_as_mapping
        nullptr,                           // tp_hash
        nullptr,                           // tp_call
        nullptr,                           // tp_str
        nullptr,                           // tp_getattro
        nullptr,                           // tp_setattro
        nullptr,                           // tp_as_buffer
        Py_TPFLAGS_DEFAULT,          // tp_flags
        "Class to store one LED pixel.",   // tp_doc
        nullptr,                           // tp_traverse
        nullptr,                           // tp_clear
        nullptr,                           // tp_richcompare
        0,                           // tp_weaklistoffset
        nullptr,                           // tp_iter
        nullptr,                           // tp_iternext
        PixelMethods,                     // tp_methods
        nullptr,                           // tp_members
        nullptr,                           // tp_getset
        nullptr,                           // tp_base
        nullptr,                           // tp_dict
        nullptr,                           // tp_descr_get
        nullptr,                           // tp_descr_set
        0,                           // tp_dictoffset
        (initproc)Pixel_init,      // tp_init
        nullptr,                           // tp_alloc
        Pixel_new,                 // tp_new
        nullptr, // tp_free
};

static PyObject* Pixel_new(PyTypeObject* type, PyObject* arg, PyObject* kw)
{
    unsigned long pixelIndex = 0;

    switch(PyTuple_Size(arg))
    {
        case 1:
            if(!PyArg_ParseTuple(arg, "k", &pixelIndex)) return nullptr;
            break;
        default:
            return nullptr;
    }

    auto self = (PixelObject*)type->tp_alloc(type, 0);
    if(!self) return nullptr;
    //Py_INCREF(self); //valgrind does not complain if we don't do this and the dealloc is only called after setting the object to "None".

    self->pixelIndex.store((uint32_t)pixelIndex, std::memory_order_release);

    return (PyObject*)self;
}

static int Pixel_init(PixelObject* self, PyObject* arg)
{
    return 0;
}

static void Pixel_dealloc(PixelObject* self)
{
	
}

static PyObject* Pixel_getFadeComplete(PixelObject* self)
{
    if(self->fadeComplete.load(std::memory_order_acquire)) return Py_True;
    else return Py_False;
}

static PyObject* Pixel_getIndex(PixelObject* self)
{
    return Py_BuildValue("k", self->pixelIndex.load(std::memory_order_acquire));
}

static PyObject* Pixel_trigger(PixelObject* self)
{
    auto rFadeComplete = self->r_akt.load(std::memory_order_acquire) == self->r_fadeTo.load(std::memory_order_acquire);
    auto gFadeComplete = self->g_akt.load(std::memory_order_acquire) == self->g_fadeTo.load(std::memory_order_acquire);
    auto bFadeComplete = self->b_akt.load(std::memory_order_acquire) == self->b_fadeTo.load(std::memory_order_acquire);
    self->fadeComplete.store(rFadeComplete && gFadeComplete && bFadeComplete, std::memory_order_release);

    PyObject* output = PyList_New(3);

    if(!self->fadeComplete.load(std::memory_order_acquire))
    {
        if(!rFadeComplete)
        {
            if(self->r_fadeDirection.load(std::memory_order_acquire) && self->r_akt.load(std::memory_order_acquire) < 255) self->r_akt++;
            else if(self->r_akt.load(std::memory_order_acquire) > 0) self->r_akt--;
        }

        if(!gFadeComplete)
        {
            if(self->g_fadeDirection.load(std::memory_order_acquire) && self->g_akt.load(std::memory_order_acquire) < 255) self->g_akt++;
            else if(self->g_akt.load(std::memory_order_acquire) > 0) self->g_akt--;
        }

        if(!bFadeComplete)
        {
            if(self->b_fadeDirection.load(std::memory_order_acquire) && self->b_akt.load(std::memory_order_acquire) < 255) self->b_akt++;
            else if(self->b_akt.load(std::memory_order_acquire) > 0) self->b_akt--;
        }
    }

    PyList_SetItem(output, 0, Py_BuildValue("b", self->r_akt.load(std::memory_order_acquire)));
    PyList_SetItem(output, 1, Py_BuildValue("b", self->g_akt.load(std::memory_order_acquire)));
    PyList_SetItem(output, 2, Py_BuildValue("b", self->b_akt.load(std::memory_order_acquire)));

    return output;
}

static PyObject* Pixel_fadeToRgb(PixelObject* self, PyObject* arg)
{
    std::array<uint8_t, 3> r{ 0, 0, 0 };

    switch(PyTuple_Size(arg))
    {
        case 1:
        {
            PyObject* rgb = nullptr;

            if(!PyArg_ParseTuple(arg, "O", &rgb) || !PyList_Check(rgb))
            {
                PyErr_SetString(PyExc_TypeError, "Argument is not of type list.");
                return nullptr;
            }

            Py_ssize_t listSize = PyList_Size(rgb);
            if(listSize != 3)
            {
                PyErr_SetString(PyExc_TypeError, "List size is not 3.");
                return nullptr;
            }

            auto value = PyList_GetItem(rgb, 0);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "First list element is not of type integer.");
                return nullptr;
            }
            r.at(0) = (uint8_t)PyLong_AsLong(value);

            value = PyList_GetItem(rgb, 1);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "Second list element is not of type integer.");
                return nullptr;
            }
            r.at(1) = (uint8_t)PyLong_AsLong(value);

            value = PyList_GetItem(rgb, 2);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "Third list element is not of type integer.");
                return nullptr;
            }
            r.at(2) = (uint8_t)PyLong_AsLong(value);

            break;
        }
        default:
            PyErr_SetString(PyExc_RuntimeError, "Invalid number of arguments.");
            return nullptr;
    }

    self->r_fadeTo.store(r.at(0), std::memory_order_release);
    self->g_fadeTo.store(r.at(1), std::memory_order_release);
    self->b_fadeTo.store(r.at(2), std::memory_order_release);
    self->fadeComplete.store(false, std::memory_order_release);
    self->r_fadeDirection.store(self->r_akt.load(std::memory_order_acquire) < self->r_fadeTo.load(std::memory_order_acquire), std::memory_order_release);
    self->g_fadeDirection.store(self->g_akt.load(std::memory_order_acquire) < self->g_fadeTo.load(std::memory_order_acquire), std::memory_order_release);
    self->b_fadeDirection.store(self->b_akt.load(std::memory_order_acquire) < self->b_fadeTo.load(std::memory_order_acquire), std::memory_order_release);

    return Py_None;
}

static PyObject* Pixel_fillRgb(PixelObject* self, PyObject* arg)
{
    std::array<uint8_t, 3> r{ 0, 0, 0 };

    switch(PyTuple_Size(arg))
    {
        case 1:
        {
            PyObject* rgb = nullptr;

            if(!PyArg_ParseTuple(arg, "O", &rgb) || !PyList_Check(rgb))
            {
                PyErr_SetString(PyExc_TypeError, "Argument is not of type list.");
                return nullptr;
            }

            Py_ssize_t listSize = PyList_Size(rgb);
            if(listSize != 3)
            {
                PyErr_SetString(PyExc_TypeError, "List size is not 3.");
                return nullptr;
            }

            auto value = PyList_GetItem(rgb, 0);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "First list element is not of type integer.");
                return nullptr;
            }
            r.at(0) = (uint8_t)PyLong_AsLong(value);

            value = PyList_GetItem(rgb, 1);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "Second list element is not of type integer.");
                return nullptr;
            }
            r.at(1) = (uint8_t)PyLong_AsLong(value);

            value = PyList_GetItem(rgb, 2);
            if(!PyLong_Check(value))
            {
                PyErr_SetString(PyExc_TypeError, "Third list element is not of type integer.");
                return nullptr;
            }
            r.at(2) = (uint8_t)PyLong_AsLong(value);

            break;
        }
        default:
            PyErr_SetString(PyExc_RuntimeError, "Invalid number of arguments.");
            return nullptr;
    }

    self->r_fadeTo.store(r.at(0), std::memory_order_release);
    self->g_fadeTo.store(r.at(1), std::memory_order_release);
    self->b_fadeTo.store(r.at(2), std::memory_order_release);
    self->r_akt.store(r.at(0), std::memory_order_release);
    self->g_akt.store(r.at(1), std::memory_order_release);
    self->b_akt.store(r.at(2), std::memory_order_release);
    self->fadeComplete.store(false, std::memory_order_release);

    return Py_None;
}

static struct PyModuleDef PixelModule = {
        PyModuleDef_HEAD_INIT,
        "ledpixelmath",   /* name of module */
        nullptr, /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
        nullptr
};

PyMODINIT_FUNC PyInit_ledpixelmath(void)
{
    PyEval_InitThreads(); //Bugfix - Not needed in Python 3.6+, no-op when called for a second time. Must be called from the main thread.

    if(PyType_Ready(&PixelObjectType) < 0) return nullptr;

    PyObject* m = PyModule_Create(&PixelModule);

    if(!m) return nullptr;

    Py_INCREF(&PixelObjectType);
    PyModule_AddObject(m, "Pixel", (PyObject*)&PixelObjectType);

    return m;
}

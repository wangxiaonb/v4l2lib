#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "v4l2lib.hpp"

PyObject *py_open(PyObject *self, PyObject *args)
{
    const char *dev;
    int width;
    int height;
    const char *colorfmt;
    PyArg_ParseTuple(args, "siis", &dev, &width, &height, &colorfmt);
    printf("camera dev: %s\n", dev);
    printf("%d x %d, %s\n", width, height, colorfmt);
    clibv4l2 *v4l2 = new clibv4l2(dev, width, height, colorfmt);
    v4l2->open();
    long result = (long)v4l2;
    // printf("py_open: %lu\n", result);
    return PyLong_FromLong(result);
}

PyObject *py_close(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->close();
    delete v4l2;
    // printf("py_close %lu\n", handle);
    return Py_None;
}

PyObject *py_start(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->start_streaming();
    // printf("py_start %lu\n", handle);
    return Py_None;
}

PyObject *py_stop(PyObject *self, PyObject *args)
{
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->stop_streaming();
    // printf("py_stop %lu\n", handle);
    return Py_None;
}

PyObject *py_read(PyObject *self, PyObject *args)
{
    void *start;
    size_t length;
    unsigned long handle;
    PyArg_ParseTuple(args, "l", &handle);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->get_frame(&start, &length);
    PyObject *object = PyMemoryView_FromMemory((char *)start, length, PyBUF_READ);
    // PyObject *result = PyByteArray_FromObject(object);
    return object;

    // PyObject *object = Py_BuildValue("y#", (char *)start, length);
    // return object;
}

PyObject *py_set_control(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u32 id;
    __s32 value;
    PyArg_ParseTuple(args, "lii", &handle, &id, &value);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    // printf("py_set_control: %lu %d %d\n", handle, id, value);
    v4l2->set_control(id, value);
    return Py_None;
}

PyObject *py_get_control(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __u32 id;
    __s32 value;
    PyArg_ParseTuple(args, "li", &handle, &id);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    value = v4l2->get_control(id);
    // printf("py_get_control: %lu %d %d\n", handle, id, value);
    return PyLong_FromLong(value);
}

PyObject *py_active_fps(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __s32 value;
    PyArg_ParseTuple(args, "li", &handle, &value);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->active_fps(value);
    // printf("py_active_fps: %lu %d\n", handle, value);
    return Py_None;
}

PyObject *py_print_fps(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __s32 value;
    PyArg_ParseTuple(args, "li", &handle, &value);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    v4l2->print_fps(value);
    // printf("py_print_fps: %lu %d\n", handle, value);
    return Py_None;
}

PyObject *py_get_fps(PyObject *self, PyObject *args)
{
    unsigned long handle;
    __s32 value;
    PyArg_ParseTuple(args, "l", &handle);
    clibv4l2 *v4l2 = (clibv4l2 *)handle;
    value = v4l2->get_fps();
    // printf("py_get_fps: %lu %d\n", handle, value);
    return PyLong_FromLong(value);
}

static PyMethodDef py_methods[] = {
    // The first property is the name exposed to Python, fast_tanh, the second is the C++
    // function name that contains the implementation.
    {"open", (PyCFunction)py_open, METH_VARARGS, nullptr},
    {"close", (PyCFunction)py_close, METH_VARARGS, nullptr},
    {"start", (PyCFunction)py_start, METH_VARARGS, nullptr},
    {"stop", (PyCFunction)py_stop, METH_VARARGS, nullptr},
    {"read", (PyCFunction)py_read, METH_VARARGS, nullptr},
    {"set_control", (PyCFunction)py_set_control, METH_VARARGS, nullptr},
    {"get_control", (PyCFunction)py_get_control, METH_VARARGS, nullptr},
    {"active_fps", (PyCFunction)py_active_fps, METH_VARARGS, nullptr},
    {"print_fps", (PyCFunction)py_print_fps, METH_VARARGS, nullptr},
    {"get_fps", (PyCFunction)py_get_fps, METH_VARARGS, nullptr},

    // Terminate the array with an object containing nulls.
    {nullptr, nullptr, 0, nullptr}};

static PyModuleDef py_module = {
    PyModuleDef_HEAD_INIT,
    "v4l2lib",                    // Module name to use with Python import statements
    "v4l2 library python driver", // Module description
    0,
    py_methods // Structure that defines the methods of the module
};

PyMODINIT_FUNC PyInit_v4l2lib()
{
    return PyModule_Create(&py_module);
}

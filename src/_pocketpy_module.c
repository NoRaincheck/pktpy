#include <Python.h>
#include <pocketpy.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int initialized = 0;

static char* importfile(const char* path, int* data_size) {
    FILE* f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(size + 1);
    if (!buf) { fclose(f); return NULL; }
    size = fread(buf, 1, size, f);
    buf[size] = 0;
    fclose(f);
    if (data_size) *data_size = (int)size;
    return buf;
}

static void print_func(const char* text) {
    PySys_WriteStdout("%s", text);
}

static void flush_func(void) {
    fflush(stdout);
}

static int getchr_func(void) {
    return getchar();
}

static void ensure_initialized(void) {
    if (!initialized) {
        py_initialize();
        py_Callbacks* cb = py_callbacks();
        cb->importfile = importfile;
        cb->print = print_func;
        cb->flush = flush_func;
        cb->getchr = getchr_func;
        initialized = 1;
    }
}

static PyObject* pocketpy_run_file(PyObject* self, PyObject* args, PyObject* kwargs) {
    const char* path;
    PyObject* argv_list = NULL;

    static char* kwlist[] = {"path", "argv", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|O", kwlist, &path, &argv_list))
        return NULL;

    ensure_initialized();

    if (argv_list && argv_list != Py_None) {
        if (!PyList_Check(argv_list)) {
            PyErr_SetString(PyExc_TypeError, "argv must be a list");
            return NULL;
        }
        Py_ssize_t argc = PyList_Size(argv_list);
        char** argv = (char**)malloc((argc + 1) * sizeof(char*));
        if (!argv) return PyErr_NoMemory();
        argv[0] = (char*)path;
        for (Py_ssize_t i = 0; i < argc; i++) {
            PyObject* s = PyObject_Str(PyList_GetItem(argv_list, i));
            if (!s) { free(argv); return NULL; }
            argv[i + 1] = (char*)PyUnicode_AsUTF8(s);
        }
        py_sys_setargv((int)(argc + 1), argv);
        free(argv);
    } else {
        char* fake_argv[] = {(char*)path};
        py_sys_setargv(1, fake_argv);
    }

    int data_size = 0;
    char* data = importfile(path, &data_size);
    if (!data) {
        PyErr_Format(PyExc_FileNotFoundError, "cannot open file '%s'", path);
        return NULL;
    }

    bool is_pyc = false;
    int len = (int)strlen(path);
    if (len >= 4 && path[len-4] == '.' && path[len-3] == 'p' && path[len-2] == 'y' && path[len-1] == 'c')
        is_pyc = true;

    bool ok = false;
    py_StackRef p0 = py_peek(0);
    if (is_pyc)
        ok = py_execo(data, data_size, path, NULL);
    else
        ok = py_exec(data, path, EXEC_MODE, NULL);

    if (!ok) {
        py_printexc();
        py_clearexc(p0);
    }

    free(data);

    if (!ok) {
        PyErr_SetString(PyExc_RuntimeError, "script raised an exception (see above)");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* pocketpy_exec(PyObject* self, PyObject* args, PyObject* kwargs) {
    const char* source;
    const char* filename = "<string>";
    int mode = EXEC_MODE;

    static char* kwlist[] = {"source", "filename", "mode", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|si", kwlist, &source, &filename, &mode))
        return NULL;

    ensure_initialized();

    bool ok = false;
    py_StackRef p0 = py_peek(0);
    ok = py_exec(source, filename, (enum py_CompileMode)mode, NULL);
    if (!ok) {
        py_printexc();
        py_clearexc(p0);
    }

    if (!ok) {
        PyErr_SetString(PyExc_RuntimeError, "execution raised an exception (see above)");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* pocketpy_eval(PyObject* self, PyObject* args) {
    const char* source;

    if (!PyArg_ParseTuple(args, "s", &source))
        return NULL;

    ensure_initialized();

    bool ok = py_eval(source, NULL);
    if (!ok) {
        char* exc = py_formatexc();
        PyErr_SetString(PyExc_RuntimeError, exc ? exc : "evaluation error");
        free(exc);
        return NULL;
    }

    py_Ref ret = py_retval();
    if (py_isnone(ret)) {
        Py_RETURN_NONE;
    } else if (py_isint(ret)) {
        return PyLong_FromLongLong(py_toint(ret));
    } else if (py_isfloat(ret)) {
        return PyFloat_FromDouble(py_tofloat(ret));
    } else if (py_isbool(ret)) {
        return PyBool_FromLong(py_tobool(ret));
    } else if (py_isstr(ret)) {
        const char* s = py_tostr(ret);
        return PyUnicode_FromString(s);
    }

    if (!py_str(ret)) {
        PyErr_SetString(PyExc_RuntimeError, "str() failed on result");
        return NULL;
    }
    py_Ref str_ret = py_retval();
    const char* s = py_tostr(str_ret);
    return PyUnicode_FromString(s ? s : "");
}

static PyObject* pocketpy_initialize(PyObject* self, PyObject* args) {
    ensure_initialized();
    Py_RETURN_NONE;
}

static PyObject* pocketpy_finalize(PyObject* self, PyObject* args) {
    if (initialized) {
        py_finalize();
        initialized = 0;
    }
    Py_RETURN_NONE;
}

static PyMethodDef PocketPyMethods[] = {
    {"initialize", pocketpy_initialize, METH_VARARGS, "Initialize the pocketpy VM"},
    {"finalize", pocketpy_finalize, METH_VARARGS, "Finalize the pocketpy VM"},
    {"run_file", (PyCFunction)pocketpy_run_file, METH_VARARGS | METH_KEYWORDS, "Run a Python file"},
    {"exec", (PyCFunction)pocketpy_exec, METH_VARARGS | METH_KEYWORDS, "Execute Python source code"},
    {"eval", pocketpy_eval, METH_VARARGS, "Evaluate a Python expression and return the result"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef pocketpy_module = {
    PyModuleDef_HEAD_INIT,
    "_pocketpy",
    "pocketpy C extension module",
    -1,
    PocketPyMethods
};

PyMODINIT_FUNC PyInit__pocketpy(void) {
    return PyModule_Create(&pocketpy_module);
}

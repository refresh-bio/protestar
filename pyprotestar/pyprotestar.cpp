#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <lib-cxx/protestar-api.h>

//binding STL container std::vector<std::string>
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);

namespace py = pybind11;


PYBIND11_MODULE(pyprotestar, m) {
	m.doc() = "Python wrapper for Proterstar API."; // optional module docstring
	
	py::bind_vector<std::vector<std::string>>(m, "StringVector");
   
	
    // Class that represents Protestar archive
    py::class_<CPSAFile>(m, "Archive")
        .def(py::init<>()) //parameterless constructor
        
        //Open(file_name) opens protestar archive
        //
        //@return true for success and false for error
        .def("Open", &CPSAFile::Open)
        
        //Close() closes opened archive
        //@return true for success and false for error
        .def("Close", &CPSAFile::Close) //Close() closes opened archive
        
       
        .def("GetNoFilesCIF", &CPSAFile::GetNoFilesCIF)
        .def("GetNoFilesPDB", &CPSAFile::GetNoFilesPDB)
        .def("GetNoFilesPAE", &CPSAFile::GetNoFilesPAE)
        .def("GetNoFilesCONF", &CPSAFile::GetNoFilesCONF)
        
        .def("ListFilesCIF", [](CPSAFile& ptr) { std::vector<std::string> out; ptr.ListFilesCIF(out); return out;})
        .def("ListFilesPDB", [](CPSAFile& ptr) { std::vector<std::string> out; ptr.ListFilesPDB(out); return out;})
        .def("ListFilesPAE", [](CPSAFile& ptr) { std::vector<std::string> out; ptr.ListFilesPAE(out); return out;})
        .def("ListFilesCONF", [](CPSAFile& ptr) { std::vector<std::string> out; ptr.ListFilesCONF(out); return out;})
        
        .def("GetFileCIF", [](CPSAFile& ptr, const std::string& filename) { std::vector<char> data; ptr.GetFileCIF(filename, data); return std::string(data.begin(), data.end());})
        .def("GetFilePDB", [](CPSAFile& ptr, const std::string& filename) { std::vector<char> data; ptr.GetFilePDB(filename, data); return std::string(data.begin(), data.end());})
        .def("GetFilePAE", [](CPSAFile& ptr, const std::string& filename) { std::vector<char> data; ptr.GetFilePAE(filename, data); return std::string(data.begin(), data.end());})
        .def("GetFileCONF",[](CPSAFile& ptr, const std::string& filename) { std::vector<char> data; ptr.GetFileCONF(filename, data); return std::string(data.begin(), data.end());});
    
    
}


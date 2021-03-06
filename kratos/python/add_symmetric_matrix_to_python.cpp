// Kratos Multi-Physics
//
// Copyright (c) 2016 Pooyan Dadvand, Riccardo Rossi, CIMNE (International Center for Numerical Methods in Engineering)
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
// 	-	Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// 	-	Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer
// 		in the documentation and/or other materials provided with the distribution.
// 	-	All advertising materials mentioning features or use of this software must display the following acknowledgement:
// 			This product includes Kratos Multi-Physics technology.
// 	-	Neither the name of the CIMNE nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDERS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED ANDON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THISSOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.



// System includes

// External includes
#include <boost/python.hpp>


// Project includes
#include "includes/define.h"
#include "includes/ublas_interface.h"
#include "python/matrix_python_interface.h"
#include "python/matrix_scalar_operator_python.h"
#include "python/matrix_scalar_assignment_operator_python.h"
#include "python/matrix_matrix_operator_python.h"

namespace Kratos
{
namespace Python
{

using namespace boost::python;


void  AddSymmetricMatrixToPython()
{

// 	  MatrixPythonInterface<symmetric_matrix<double, upper>, upper>::CreateInterface("SymmetricMatrix")
// 		  .def(init<symmetric_matrix<double, upper>::size_type>())
// 		  .def(init<symmetric_matrix<double, upper>::size_type, symmetric_matrix<double, upper>::size_type>())
// 		  .def(MatrixScalarOperatorPython<symmetric_matrix<double, upper>, double>())
// 		  .def(MatrixScalarAssignmentOperatorPython<symmetric_matrix<double, upper>, double>())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, zero_matrix<double>, symmetric_matrix<double, upper> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, identity_matrix<double>, symmetric_matrix<double, upper> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, scalar_matrix<double>, matrix<double> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, banded_matrix<double>, matrix<double> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, triangular_matrix<double, upper>, matrix<double> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, triangular_matrix<double, lower>, matrix<double> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, matrix<double>, matrix<double> >())
// #if defined KRATOS_ADD_HERMITIAN_MATRIX_INTERFACE
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, hermitian_matrix<double, upper>, matrix<double> >())
// #endif
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, mapped_matrix<double>, matrix<double> >())
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, compressed_matrix<double>, matrix<double> >())
// #if defined KRATOS_ADD_COORDINATE_MATRIX_INTERFACE
// 	          .def(MatrixMatrixOperatorPython<symmetric_matrix<double, upper>, coordinate_matrix<double>, matrix<double> >())
// #endif
// 		  ;

}

}  // namespace Python.

} // Namespace Kratos


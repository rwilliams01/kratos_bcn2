//    |  /           |
//    ' /   __| _` | __|  _ \   __|
//    . \  |   (   | |   (   |\__ `
//   _|\_\_|  \__,_|\__|\___/ ____/
//                   Multi-Physics
//
//  License:         BSD License
//                     Kratos default license: kratos/license.txt
//
//  Main authors:    hbui
//
//
#if !defined(KRATOS_RESIDUAL_BASED_BLOCK_BUILDER_AND_SOLVER_WITH_CONSTRAINTS_DEACTIVATION )
#define  KRATOS_RESIDUAL_BASED_BLOCK_BUILDER_AND_SOLVER_WITH_CONSTRAINTS_DEACTIVATION


/* System includes */
#include <iostream>
#include <unordered_set>
#include <unordered_map>

/* External includes */
#include "boost/progress.hpp"

/* Project includes */
#include "includes/define.h"
#include "solving_strategies/builder_and_solvers/builder_and_solver.h"
#include "includes/model_part.h"
#include "utilities/timer.h"
#include "utilities/openmp_utils.h"
#include "includes/kratos_flags.h"
#include "utilities/sparse_matrix_multiplication_utility.h"


namespace Kratos
{

///@name Kratos Globals
///@{

///@}
///@name Type Definitions
///@{

///@}
///@name  Enum's
///@{

///@}
///@name  Functions
///@{

///@}
///@name Kratos Classes
///@{

/**
 * @class ResidualBasedEliminationBuilderAndSolver
 * @ingroup KratosCore
 * @brief Current class provides an implementation for standard builder and solving operations.
 * @details The RHS is constituted by the unbalanced loads (residual)
 * Degrees of freedom are reordered putting the restrained degrees of freedom at
 * the end of the system ordered in reverse order with respect to the DofSet.
 * Imposition of the dirichlet conditions is naturally dealt with as the residual already contains
 * this information.
 * Calculation of the reactions involves a cost very similiar to the calculation of the total residual
 * @author Riccardo Rossi
 */
template<class TSparseSpace,
         class TDenseSpace, //= DenseSpace<double>,
         class TLinearSolver //= LinearSolver<TSparseSpace,TDenseSpace>
         >
class ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation
    : public BuilderAndSolver< TSparseSpace, TDenseSpace, TLinearSolver >
{
public:
    ///@name Type Definitions
    ///@{
    KRATOS_CLASS_POINTER_DEFINITION(ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation);

    /// Definition of the base class
    typedef BuilderAndSolver<TSparseSpace, TDenseSpace, TLinearSolver> BaseType;

    // The size_t types
    typedef std::size_t SizeType;
    typedef std::size_t IndexType;

    /// Definition of the classes from the base class
    typedef typename BaseType::TSchemeType TSchemeType;
    typedef typename BaseType::TDataType TDataType;
    typedef typename BaseType::DofsArrayType DofsArrayType;
    typedef typename BaseType::TSystemMatrixType TSystemMatrixType;
    typedef typename BaseType::TSystemVectorType TSystemVectorType;
    typedef typename BaseType::LocalSystemVectorType LocalSystemVectorType;
    typedef typename BaseType::LocalSystemMatrixType LocalSystemMatrixType;
    typedef typename BaseType::TSystemMatrixPointerType TSystemMatrixPointerType;
    typedef typename BaseType::TSystemVectorPointerType TSystemVectorPointerType;
    typedef typename BaseType::NodesArrayType NodesArrayType;
    typedef typename BaseType::ElementsArrayType ElementsArrayType;
    typedef typename BaseType::ConditionsArrayType ConditionsArrayType;

    /// Additional definitions
    typedef PointerVectorSet<Element, IndexedObject> ElementsContainerType;
    typedef Element::EquationIdVectorType EquationIdVectorType;
    typedef Element::DofsVectorType DofsVectorType;
    typedef boost::numeric::ublas::compressed_matrix<double> CompressedMatrixType;

    /// DoF types definition
    typedef Node<3> NodeType;
    typedef typename NodeType::DofType DofType;
    typedef typename DofType::Pointer DofPointerType;

    ///@}
    ///@name Life Cycle
    ///@{

    /**
     * @brief Default constructor. (with parameters)
     */
    // explicit ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation(
    //     typename TLinearSolver::Pointer pNewLinearSystemSolver,
    //     Parameters ThisParameters
    //     ) : BaseType(pNewLinearSystemSolver)
    // {
    //     // Validate default parameters
    //     Parameters default_parameters = Parameters(R"(
    //     {
    //         "name" : "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation"
    //     })" );
    //
    //     ThisParameters.ValidateAndAssignDefaults(default_parameters);
    // }

    /**
     * @brief Default constructor.
     */
    explicit ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation(
        typename TLinearSolver::Pointer pNewLinearSystemSolver)
        : BaseType(pNewLinearSystemSolver)
    {
    }

    /** Destructor.
     */
    ~ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation() //override
    {
    }

    ///@}
    ///@name Operators
    ///@{

    ///@}
    ///@name Operations
    ///@{

    /**
     * @brief Function to perform the build of the RHS. The vector could be sized as the total number
     * of dofs or as the number of unrestrained ones
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     * @param b The RHS vector
     */
    void Build(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& b) //override
    {
        KRATOS_TRY

        if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Entering Build" << std::endl;
        }

        if(!pScheme)
        {
            KRATOS_THROW_ERROR(std::logic_error, "No scheme provided!", "");
        }
        // Getting the elements from the model
        const int nelements = static_cast<int>(rModelPart.Elements().size());

        // Getting the array of the conditions
        const int nconditions = static_cast<int>(rModelPart.Conditions().size());

        std::cout << "Total number of elements in assembly: " << nelements << std::endl;
        std::cout << "Total number of conditions in assembly: " << nconditions << std::endl;
        std::cout << "Total number of constraints in assembly: " << rModelPart.NumberOfMasterSlaveConstraints() << std::endl;
        std::cout << "Number of threads: " << OpenMPUtils::GetNumThreads() << std::endl;

        ProcessInfo& CurrentProcessInfo = rModelPart.GetProcessInfo();
        ModelPart::ElementsContainerType::iterator el_begin = rModelPart.ElementsBegin();
        ModelPart::ConditionsContainerType::iterator cond_begin = rModelPart.ConditionsBegin();

        //contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        //vector containing the localization in the system of the different
        //terms
        Element::EquationIdVectorType EquationId;

        // assemble all elements
        double start_build = OpenMPUtils::GetCurrentTime();

        #pragma omp parallel firstprivate(nelements,nconditions, LHS_Contribution, RHS_Contribution, EquationId )
        {
            # pragma omp for  schedule(guided, 512) nowait
            for (int k = 0; k < nelements; k++)
            {
                ModelPart::ElementsContainerType::iterator it = el_begin + k;

                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool element_is_active = true;
                if ((it)->IsDefined(ACTIVE))
                    element_is_active = (it)->Is(ACTIVE);

                if (element_is_active)
                {
                    //calculate elemental contribution
                    pScheme->CalculateSystemContributions(*(it.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution
                    Assemble(A, b, LHS_Contribution, RHS_Contribution, EquationId);

                    // clean local elemental memory
                    pScheme->CleanMemory(*(it.base()));
                }
            }

            #pragma omp for  schedule(guided, 512)
            for (int k = 0; k < nconditions; k++)
            {
                ModelPart::ConditionsContainerType::iterator it = cond_begin + k;

                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool condition_is_active = true;
                if ((it)->IsDefined(ACTIVE))
                    condition_is_active = (it)->Is(ACTIVE);

                if (condition_is_active)
                {
                    //calculate elemental contribution
                    pScheme->Condition_CalculateSystemContributions(*(it.base()), LHS_Contribution, RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution
                    Assemble(A, b, LHS_Contribution, RHS_Contribution, EquationId);

                    // clean local elemental memory
                    pScheme->CleanMemory(*(it.base()));
                }
            }
        }

        const double stop_build = OpenMPUtils::GetCurrentTime();
        if (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Build time: " << stop_build - start_build << std::endl;
        }

        if (this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Finished parallel building" << std::endl;
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the building of the LHS
     * @details Depending on the implementation choosen the size of the matrix could
     * be equal to the total number of Dofs or to the number of unrestrained dofs
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     */
    void BuildLHS(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A) //override
    {
        KRATOS_TRY

        TSystemVectorType tmp(A.size1(), 0.0);
        this->Build(pScheme, rModelPart, A, tmp);

        KRATOS_CATCH("")
    }

    /**
     * @brief Build a rectangular matrix of size n*N where "n" is the number of unrestrained degrees of freedom
     * and "N" is the total number of degrees of freedom involved.
     * @details This matrix is obtained by building the total matrix without the lines corresponding to the fixed
     * degrees of freedom (but keeping the columns!!)
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     */
    void BuildLHS_CompleteOnFreeRows(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A) //override
    {
        KRATOS_TRY

        TSystemVectorType tmp(A.size1(), 0.0);
        this->Build(pScheme, rModelPart, A, tmp);

        KRATOS_CATCH("")
    }

    /**
     * @brief This is a call to the linear system solver
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void SystemSolve(
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b
    ) //override
    {
        KRATOS_TRY

        double start_solve = OpenMPUtils::GetCurrentTime();

        double norm_b;
        if (TSparseSpace::Size(b) != 0)
            norm_b = TSparseSpace::TwoNorm(b);
        else
            norm_b = 0.00;

        if (norm_b != 0.00)
        {
            //do solve
            BaseType::mpLinearSystemSolver->Solve(A, Dx, b);
        }
        else
            TSparseSpace::SetToZero(Dx);

        if(mT.size1() != 0) //if there are master-slave constraints
        {
            //recover solution of the original problem
            TSystemVectorType Dxmodified = Dx;

            TSparseSpace::Mult(mT, Dxmodified, Dx);
        }

        double stop_solve = OpenMPUtils::GetCurrentTime();
        std::cout << "System Solve time: " << stop_solve - start_solve << "s" << std::endl;

        //prints informations about the current time
        if (this->GetEchoLevel() > 1)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << *(BaseType::mpLinearSystemSolver) << std::endl;
        }

        KRATOS_CATCH("")
    }

    void SystemSolveWithPhysics(
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b,
        ModelPart& rModelPart
    )
    {
        if(rModelPart.MasterSlaveConstraints().size() != 0) {
            TSystemVectorType Dxmodified(b.size());
            InternalSystemSolveWithPhysics(A, Dxmodified, b, rModelPart);

            //recover solution of the original problem
            double start_recover = OpenMPUtils::GetCurrentTime();
            TSparseSpace::Mult(mT, Dxmodified, Dx);
            double stop_recover = OpenMPUtils::GetCurrentTime();
            std::cout << "Recover back the system time: " << stop_recover - start_recover << "s" << std::endl;
        } else {
            InternalSystemSolveWithPhysics(A, Dx, b, rModelPart);
        }
    }

    /**
      *@brief This is a call to the linear system solver (taking into account some physical particularities of the problem)
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     * @param rModelPart The model part of the problem to solve
     */
    void InternalSystemSolveWithPhysics(
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b,
        ModelPart& rModelPart
    )
    {
        KRATOS_TRY

        double start_solve = OpenMPUtils::GetCurrentTime();
        std::cout << "Begin Internal-System-Solve-With-Physics" << std::endl;

        double norm_b;
        if (TSparseSpace::Size(b) != 0)
            norm_b = TSparseSpace::TwoNorm(b);
        else
            norm_b = 0.00;

        if (norm_b != 0.00) {
            //provide physical data as needed
            if(BaseType::mpLinearSystemSolver->AdditionalPhysicalDataIsNeeded() )
                BaseType::mpLinearSystemSolver->ProvideAdditionalData(A, Dx, b, BaseType::mDofSet, rModelPart);

            // double norm_bu = 0, norm_bp = 0;
            // double norm_dxu = 0, norm_dxp = 0;
            // double norm_diag_au = 0.0, norm_diag_ap = 0.0;
            // std::size_t n_pressure_dofs = 0;
            // for (auto dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator)
            // {
            //     // std::cout << "node " << dof_iterator->Id()
            //     //           << " EquationId: " << dof_iterator->EquationId()
            //     //           << ", " << dof_iterator->GetVariable().Name() << ": " << dof_iterator->GetSolutionStepValue()
            //     //           << std::endl;
            //     if (dof_iterator->GetVariable().Name() == "WATER_PRESSURE")
            //     {
            //         norm_bp += pow(b[dof_iterator->EquationId()], 2);
            //         norm_dxp += pow(Dx[dof_iterator->EquationId()], 2);
            //         norm_diag_ap += pow(A(dof_iterator->EquationId(), dof_iterator->EquationId()), 2);
            //         ++n_pressure_dofs;
            //         // std::cout << "node " << dof_iterator->Id()
            //         //       << " EquationId: " << dof_iterator->EquationId()
            //         //       << ", " << dof_iterator->GetVariable().Name() << ": " << dof_iterator->GetSolutionStepValue()
            //         //       << ", IsFixed: " << dof_iterator->IsFixed()
            //         //       << std::endl;
            //     }
            //     else
            //     {
            //         norm_bu += pow(b[dof_iterator->EquationId()], 2);
            //         norm_dxu += pow(Dx[dof_iterator->EquationId()], 2);
            //         norm_diag_au += pow(A(dof_iterator->EquationId(), dof_iterator->EquationId()), 2);
            //     }
            // }
            // norm_bu = sqrt(norm_bu);
            // norm_bp = sqrt(norm_bp);
            // norm_dxu = sqrt(norm_dxu);
            // norm_dxp = sqrt(norm_dxp);
            // norm_diag_au = sqrt(norm_diag_au);
            // norm_diag_ap = sqrt(norm_diag_ap);

            // KRATOS_WATCH(__LINE__)
            // std::cout << "Before solve:" << std::endl;
            // KRATOS_WATCH(norm_bu)
            // KRATOS_WATCH(norm_bp)
            // KRATOS_WATCH(norm_dxu)
            // KRATOS_WATCH(norm_dxp)
            // KRATOS_WATCH(norm_diag_au)
            // KRATOS_WATCH(norm_diag_ap)
            // KRATOS_WATCH(n_pressure_dofs)

            //do solve
            BaseType::mpLinearSystemSolver->Solve(A, Dx, b);

            // for (auto dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator)
            // {
            //     std::cout << "node " << dof_iterator->Id()
            //               << " EquationId: " << dof_iterator->EquationId()
            //               << ", " << dof_iterator->GetVariable().Name() << ": " << dof_iterator->GetSolutionStepValue()
            //               << std::endl;
            // }

            // norm_bu = 0; norm_bp = 0;
            // norm_dxu = 0; norm_dxp = 0;
            // for (auto dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator)
            // {
            //     // std::cout << "node " << dof_iterator->Id()
            //     //           << " EquationId: " << dof_iterator->EquationId()
            //     //           << ", " << dof_iterator->GetVariable().Name() << ": " << dof_iterator->GetSolutionStepValue()
            //     //           << std::endl;
            //     if (dof_iterator->GetVariable().Name() == "WATER_PRESSURE")
            //     {
            //         norm_bp += pow(b[dof_iterator->EquationId()], 2);
            //         norm_dxp += pow(Dx[dof_iterator->EquationId()], 2);
            //     }
            //     else
            //     {
            //         norm_bu += pow(b[dof_iterator->EquationId()], 2);
            //         norm_dxu += pow(Dx[dof_iterator->EquationId()], 2);
            //     }
            // }
            // norm_bu = sqrt(norm_bu);
            // norm_bp = sqrt(norm_bp);
            // norm_dxu = sqrt(norm_dxu);
            // norm_dxp = sqrt(norm_dxp);

            // KRATOS_WATCH(__LINE__)
            // std::cout << "After solve:" << std::endl;
            // KRATOS_WATCH(norm_bu)
            // KRATOS_WATCH(norm_bp)
            // KRATOS_WATCH(norm_dxu)
            // KRATOS_WATCH(norm_dxp)
        } else {
            TSparseSpace::SetToZero(Dx);
            std::cout << "ATTENTION! setting the RHS to zero!" << std::endl;
            // KRATOS_WARNING("ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation") << "ATTENTION! setting the RHS to zero!" << std::endl;
        }

        double stop_solve = OpenMPUtils::GetCurrentTime();
        std::cout << "Internal-System-Solve-With-Physics time: " << stop_solve - start_solve << "s" << std::endl;

        // Prints informations about the current time
        if (this->GetEchoLevel() > 1)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << *(BaseType::mpLinearSystemSolver) << std::endl;
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the building and solving phase at the same time.
     * @details It is ideally the fastest and safer function to use when it is possible to solve
     * just after building
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void BuildAndSolve(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b) //override
    {
        KRATOS_TRY

        Timer::Start("Build");

        Build(pScheme, rModelPart, A, b);

        Timer::Stop("Build");

        if(rModelPart.MasterSlaveConstraints().size() != 0) {
            Timer::Start("ApplyConstraints");
            ApplyConstraints(pScheme,A,Dx,b,rModelPart);
            Timer::Stop("ApplyConstraints");
        }

        ApplyDirichletConditions(pScheme, rModelPart, A, Dx, b);

        if ( this->GetEchoLevel() > 3)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Before the solution of the system" << "\nSystem Matrix = " << A << "\nUnknowns vector = " << Dx << "\nRHS vector = " << b << std::endl;
        }

        const double start_solve = OpenMPUtils::GetCurrentTime();
        Timer::Start("Solve");

        SystemSolveWithPhysics(A, Dx, b, rModelPart);

        Timer::Stop("Solve");
        const double stop_solve = OpenMPUtils::GetCurrentTime();

        if (this->GetEchoLevel() >=1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "System solve time: " << stop_solve - start_solve << std::endl;
        }

        if ( this->GetEchoLevel() > 3)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "After the solution of the system" << "\nSystem Matrix = " << A << "\nUnknowns vector = " << Dx << "\nRHS vector = " << b << std::endl;
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief Corresponds to the previews, but the System's matrix is considered already built and only the RHS is built again
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void BuildRHSAndSolve(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b) //override
    {
        KRATOS_TRY

        BuildRHS(pScheme, rModelPart, b);
        SystemSolve(A, Dx, b);

        KRATOS_CATCH("")
    }

    /**
     * @brief Function to perform the build of the RHS.
     * @details The vector could be sized as the total number of dofs or as the number of unrestrained ones
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     */
    void BuildRHS(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemVectorType& b) //override
    {
        KRATOS_TRY

        BuildRHSNoDirichlet(pScheme,rModelPart,b);

        const int ndofs = static_cast<int>(BaseType::mDofSet.size());

        //NOTE: dofs are assumed to be numbered consecutively in the BlockBuilderAndSolver
        #pragma omp parallel for firstprivate(ndofs)
        for (int k = 0; k<ndofs; k++)
        {
            typename DofsArrayType::iterator dof_iterator = BaseType::mDofSet.begin() + k;
            const std::size_t i = dof_iterator->EquationId();

            if (dof_iterator->IsFixed())
                b[i] = 0.0;
        }

        KRATOS_CATCH("")
    }

    /**
     * @brief Builds the list of the DofSets involved in the problem by "asking" to each element
     * and condition its Dofs.
     * @details The list of dofs is stores insde the BuilderAndSolver as it is closely connected to the
     * way the matrix and RHS are built
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     */
    void SetUpDofSet(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart
    ) //override
    {
        KRATOS_TRY;

        if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Setting up the dofs" << std::endl;
        }

        DofsVectorType dof_list, second_dof_list; // NOTE: The second dof list is only used on constraints to include master/slave relations

        ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        DofsArrayType Doftemp;
        mAllDofs.clear();

        // Gets the array of elements from the modeler
        ElementsArrayType& r_elements_array = rModelPart.Elements();
        const std::size_t number_of_elements = r_elements_array.size();
        bool element_is_active;
        for(typename ElementsArrayType::ptr_iterator it = r_elements_array.ptr_begin(); it != r_elements_array.ptr_end(); ++it)
        {
            // Gets list of Dof involved on every element
            pScheme->GetElementalDofList((*it), dof_list, r_current_process_info);

            for(typename DofsVectorType::iterator i = dof_list.begin() ; i != dof_list.end() ; ++i)
            {
                #if defined(KRATOS_SD_REF_NUMBER_2)
                mAllDofs.push_back(*i);
                #elif defined(KRATOS_SD_REF_NUMBER_3)
                mAllDofs.push_back(i->get());
                #endif
            }

            element_is_active = true;
            if( (*it)->IsDefined(ACTIVE) ) {
                element_is_active = (*it)->Is(ACTIVE);
            }

            if (element_is_active)
            {
                for(typename DofsVectorType::iterator i = dof_list.begin() ; i != dof_list.end() ; ++i)
                {
                    #if defined(KRATOS_SD_REF_NUMBER_2)
                    Doftemp.push_back(*i);
                    #elif defined(KRATOS_SD_REF_NUMBER_3)
                    Doftemp.push_back(i->get());
                    #endif
                }
            }
        }

        // Gets the array of conditions from the modeler
        ConditionsArrayType& r_conditions_array = rModelPart.Conditions();
        const std::size_t number_of_conditions = r_conditions_array.size();
        bool condition_is_active;
        for(typename ConditionsArrayType::ptr_iterator it = r_conditions_array.ptr_begin(); it != r_conditions_array.ptr_end(); ++it)
        {
            // Gets list of Dof involved on every condition
            pScheme->GetConditionDofList((*it), dof_list, r_current_process_info);

            for(typename DofsVectorType::iterator i = dof_list.begin() ; i != dof_list.end() ; ++i)
            {
                #if defined(KRATOS_SD_REF_NUMBER_2)
                mAllDofs.push_back(*i);
                #elif defined(KRATOS_SD_REF_NUMBER_3)
                mAllDofs.push_back(i->get());
                #endif
            }

            condition_is_active = true;
            if( (*it)->IsDefined(ACTIVE) ) {
                condition_is_active = (*it)->Is(ACTIVE);
            }

            if (condition_is_active)
            {
                for(typename DofsVectorType::iterator i = dof_list.begin() ; i != dof_list.end() ; ++i)
                {
                    #if defined(KRATOS_SD_REF_NUMBER_2)
                    Doftemp.push_back(*i);
                    #elif defined(KRATOS_SD_REF_NUMBER_3)
                    Doftemp.push_back(i->get());
                    #endif
                }
            }
        }

        Doftemp.Unique();

        BaseType::mDofSet = Doftemp;

        mAllDofs.Unique();

        //Throws an exception if there are no Degrees Of Freedom involved in the analysis
        if(BaseType::mDofSet.size() == 0)
        {
            KRATOS_THROW_ERROR(std::logic_error, "No degrees of freedom!", "");
        }

        if ( this->GetEchoLevel() > 2)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Number of degrees of freedom: " << BaseType::mDofSet.size() << std::endl;
        }

        BaseType::mDofSetIsInitialized = true;

        if ( this->GetEchoLevel() > 2 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Finished setting up the dofs" << std::endl;
        }

        if ( this->GetEchoLevel() > 2)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "End of setup dof set\n" << std::endl;
        }


#ifdef KRATOS_DEBUG
        // If reactions are to be calculated, we check if all the dofs have reactions defined
        // This is tobe done only in debug mode
        if (BaseType::GetCalculateReactionsFlag()) {
            for (auto dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator) {
                if(!(dof_iterator->HasReaction()))
                {
                    KRATOS_THROW_ERROR(std::logic_error, "Reaction variable not set for the following Node :xxx Not possible to calculate reactions.", dof_iterator->Id());
                }
            }
        }
#endif

        KRATOS_CATCH("");
    }

    /**
     * @brief Organises the dofset in order to speed up the building phase
     * @param rModelPart The model part of the problem to solve
     */
    void SetUpSystem(
        ModelPart& rModelPart
    ) //override
    {
        if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Entering SetUpSystem" << std::endl;
        }

        // initialize the equation id for every dofs in the system. This is to avoid any untouched dofs when the elements are deactivated.
        for (typename DofsArrayType::iterator dof_iterator = mAllDofs.begin(); dof_iterator != mAllDofs.end(); ++dof_iterator)
        {
            dof_iterator->SetEquationId(-1);
        }

        BaseType::mEquationSystemSize = BaseType::mDofSet.size();

        std::size_t free_id = 0;
        for (typename DofsArrayType::iterator dof_iterator = BaseType::mDofSet.begin(); dof_iterator != BaseType::mDofSet.end(); ++dof_iterator)
        {
            dof_iterator->SetEquationId(free_id++);
        }
    }

    //**************************************************************************
    //**************************************************************************

    void ResizeAndInitializeVectors(
        TSystemMatrixPointerType& pA,
        TSystemVectorPointerType& pDx,
        TSystemVectorPointerType& pb,
        ModelPart& rModelPart
    ) ////override
    {
        if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Entering ResizeAndInitializeVectors" << std::endl;
        }

        KRATOS_TRY

        if (pA == NULL) //if the pointer is not initialized initialize it to an empty matrix
        {
            TSystemMatrixPointerType pNewA = TSystemMatrixPointerType(new TSystemMatrixType(0, 0));
            pA.swap(pNewA);
        }
        if (pDx == NULL) //if the pointer is not initialized initialize it to an empty matrix
        {
            TSystemVectorPointerType pNewDx = TSystemVectorPointerType(new TSystemVectorType(0));
            pDx.swap(pNewDx);
        }
        if (pb == NULL) //if the pointer is not initialized initialize it to an empty matrix
        {
            TSystemVectorPointerType pNewb = TSystemVectorPointerType(new TSystemVectorType(0));
            pb.swap(pNewb);
        }
        if (BaseType::mpReactionsVector == NULL) //if the pointer is not initialized initialize it to an empty matrix
        {
            TSystemVectorPointerType pNewReactionsVector = TSystemVectorPointerType(new TSystemVectorType(0) );
            BaseType::mpReactionsVector.swap(pNewReactionsVector);
        }

        TSystemMatrixType& A = *pA;
        TSystemVectorType& Dx = *pDx;
        TSystemVectorType& b = *pb;

        //resizing the system vectors and matrix
        if (A.size1() == 0 || BaseType::GetReshapeMatrixFlag() == true) //if the matrix is not initialized
        {
            A.resize(BaseType::mEquationSystemSize, BaseType::mEquationSystemSize, false);
            ConstructMatrixStructure(A, rModelPart);
        }
        else
        {
            if (A.size1() != BaseType::mEquationSystemSize || A.size2() != BaseType::mEquationSystemSize)
            {
                KRATOS_THROW_ERROR(std::logic_error, "The equation system size has changed during the simulation. This is not permited.", "");
                A.resize(BaseType::mEquationSystemSize, BaseType::mEquationSystemSize, true);
                ConstructMatrixStructure(A, rModelPart);
            }
        }

        if (Dx.size() != BaseType::mEquationSystemSize)
            Dx.resize(BaseType::mEquationSystemSize, false);
        if (b.size() != BaseType::mEquationSystemSize)
            b.resize(BaseType::mEquationSystemSize, false);

        //if needed resize the vector for the calculation of reactions
        if(BaseType::mCalculateReactionsFlag == true)
        {
            unsigned int ReactionsVectorSize = BaseType::mDofSet.size()-BaseType::mEquationSystemSize;
            if(BaseType::mpReactionsVector->size() != ReactionsVectorSize)
                BaseType::mpReactionsVector->resize(ReactionsVectorSize,false);
        }

        ConstructMasterSlaveConstraintsStructure(rModelPart);

        KRATOS_CATCH("")
    }

    //**************************************************************************
    //**************************************************************************

    void InitializeSolutionStep(
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb) //override
    {
        KRATOS_TRY

        BaseType::InitializeSolutionStep(rModelPart, rA, rDx, rb);

        // Getting process info
        const ProcessInfo& r_process_info = rModelPart.GetProcessInfo();

        // Computing constraints
        const int n_constraints = static_cast<int>(rModelPart.MasterSlaveConstraints().size());
        auto constraints_begin = rModelPart.MasterSlaveConstraintsBegin();
        #pragma omp parallel for schedule(guided, 512) firstprivate(n_constraints, constraints_begin)
        for (int k = 0; k < n_constraints; ++k) {
            auto it = constraints_begin + k;
            it->InitializeSolutionStep(r_process_info); // Here each constraint constructs and stores its T and C matrices. Also its equation slave_ids.
        }

        KRATOS_CATCH("")
    }

    //**************************************************************************
    //**************************************************************************

    void FinalizeSolutionStep(
        ModelPart& rModelPart,
        TSystemMatrixType& rA,
        TSystemVectorType& rDx,
        TSystemVectorType& rb) //override
    {
        BaseType::FinalizeSolutionStep(rModelPart, rA, rDx, rb);

        // Getting process info
        const ProcessInfo& r_process_info = rModelPart.GetProcessInfo();

        // Computing constraints
        const int n_constraints = static_cast<int>(rModelPart.MasterSlaveConstraints().size());
        const auto constraints_begin = rModelPart.MasterSlaveConstraintsBegin();
        #pragma omp parallel for schedule(guided, 512) firstprivate(n_constraints, constraints_begin)
        for (int k = 0; k < n_constraints; ++k) {
            auto it = constraints_begin + k;
            it->FinalizeSolutionStep(r_process_info);
        }
    }

    //**************************************************************************
    //**************************************************************************

    void CalculateReactions(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b) //override
    {
        TSparseSpace::SetToZero(b);

        //refresh RHS to have the correct reactions
        BuildRHSNoDirichlet(pScheme, rModelPart, b);

        const int ndofs = static_cast<int>(BaseType::mDofSet.size());

        //NOTE: dofs are assumed to be numbered consecutively in the BlockBuilderAndSolver
        #pragma omp parallel for firstprivate(ndofs)
        for (int k = 0; k<ndofs; k++) {
            typename DofsArrayType::iterator dof_iterator = BaseType::mDofSet.begin() + k;

            const int i = (dof_iterator)->EquationId();
            (dof_iterator)->GetSolutionStepReactionValue() = -b[i];
        }
    }

    /**
     * @brief Applies the dirichlet conditions. This operation may be very heavy or completely
     * unexpensive depending on the implementation choosen and on how the System Matrix is built.
     * @details For explanation of how it works for a particular implementation the user
     * should refer to the particular Builder And Solver choosen
     * @param pScheme The integration scheme considered
     * @param rModelPart The model part of the problem to solve
     * @param A The LHS matrix
     * @param Dx The Unknowns vector
     * @param b The RHS vector
     */
    void ApplyDirichletConditions(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemMatrixType& A,
        TSystemVectorType& Dx,
        TSystemVectorType& b) //override
    {
        const double start_apply = OpenMPUtils::GetCurrentTime();

        std::size_t system_size = A.size1();
        std::vector<double> scaling_factors (system_size, 0.0);

        const int ndofs = static_cast<int>(BaseType::mDofSet.size());

        //NOTE: dofs are assumed to be numbered consecutively in the BlockBuilderAndSolver
        #pragma omp parallel for firstprivate(ndofs)
        for (int k = 0; k<ndofs; k++) {
            typename DofsArrayType::iterator dof_iterator = BaseType::mDofSet.begin() + k;
            if(dof_iterator->IsFixed())
                scaling_factors[k] = 0.0;
            else
                scaling_factors[k] = 1.0;

        }

        double* Avalues = A.value_data().begin();
        std::size_t* Arow_indices = A.index1_data().begin();
        std::size_t* Acol_indices = A.index2_data().begin();

        //detect if there is a line of all zeros and set the diagonal to a 1 if this happens
        #pragma omp parallel for firstprivate(system_size)
        for (int k = 0; k < static_cast<int>(system_size); ++k){
            std::size_t col_begin = Arow_indices[k];
            std::size_t col_end = Arow_indices[k+1];
            bool empty = true;
            for (std::size_t j = col_begin; j < col_end; ++j)
            {
                if(Avalues[j] != 0.0)
                {
                    empty = false;
                    break;
                }
            }

            if(empty == true)
            {
                A(k,k) = 1.0;
                b[k] = 0.0;
            }
        }

        #pragma omp parallel for
        for (int k = 0; k < static_cast<int>(system_size); ++k)
        {
            std::size_t col_begin = Arow_indices[k];
            std::size_t col_end = Arow_indices[k+1];
            double k_factor = scaling_factors[k];
            if (k_factor == 0)
            {
                // zero out the whole row, except the diagonal
                for (std::size_t j = col_begin; j < col_end; ++j)
                    if (static_cast<int>(Acol_indices[j]) != k )
                        Avalues[j] = 0.0;

                // zero out the RHS
                b[k] = 0.0;
            }
            else
            {
                // zero out the column which is associated with the zero'ed row
                for (std::size_t j = col_begin; j < col_end; ++j)
                    if(scaling_factors[ Acol_indices[j] ] == 0 )
                        Avalues[j] = 0.0;
            }
        }

        const double stop_apply = OpenMPUtils::GetCurrentTime();

        if (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Apply Dirichlet time: " << stop_apply - start_apply << std::endl;
        }
    }

    /**
     * @brief This function is intended to be called at the end of the solution step to clean up memory storage not needed
     */
    void Clear() //override
    {
        BaseType::Clear();
        this->mpLinearSystemSolver->Clear();

        mSlaveIds.clear();
        mMasterIds.clear();
        mInactiveSlaveDofs.clear();
        mT.resize(0,0,false);
        mConstantVector.resize(0,false);

        if (this->GetEchoLevel() > 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation Clear Function called" << std::endl;
        }
    }

    /**
     * @brief This function is designed to be called once to perform all the checks needed
     * on the input provided. Checks can be "expensive" as the function is designed
     * to catch user's errors.
     * @param rModelPart The model part of the problem to solve
     * @return 0 all ok
     */
    int Check(ModelPart& rModelPart) //override
    {
        KRATOS_TRY

        return 0;
        KRATOS_CATCH("");
    }

    ///@}
    ///@name Access
    ///@{

    ///@}
    ///@name Inquiry
    ///@{

    ///@}
    ///@name Input and output
    ///@{

    /// Turn back information as a string.
    std::string Info() const //override
    {
        return "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation";
    }

    /// Print information about this object.
    void PrintInfo(std::ostream& rOStream) const //override
    {
        rOStream << Info();
    }

    /// Print object's data.
    void PrintData(std::ostream& rOStream) const //override
    {
        rOStream << Info();
    }

    ///@}
    ///@name Friends
    ///@{

    ///@}

protected:
    ///@name Protected static Member Variables
    ///@{

    ///@}
    ///@name Protected member Variables
    ///@{

    TSystemMatrixType mT;              /// This is matrix containing the global relation for the constraints
    TSystemVectorType mConstantVector; /// This is vector containing the rigid movement of the constraint
    std::vector<IndexType> mSlaveIds;  /// The equation ids of the slaves
    std::vector<IndexType> mMasterIds; /// The equation ids of the master
    std::unordered_set<IndexType> mInactiveSlaveDofs; /// The set containing the inactive slave dofs

    ///@}
    ///@name Protected Operators
    ///@{

    ///@}
    ///@name Protected Operations
    ///@{

    void ConstructMasterSlaveConstraintsStructure(ModelPart& rModelPart)
    {
        if (rModelPart.MasterSlaveConstraints().size() > 0) {

            if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
            {
                std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Entering ConstructMasterSlaveConstraintsStructure" << std::endl;
            }

            const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

            // Vector containing the localization in the system of the different terms
            DofsVectorType slave_dof_list, master_dof_list;

            // Constraint initial iterator
            const auto it_const_begin = rModelPart.MasterSlaveConstraints().begin();
            std::vector<std::unordered_set<IndexType> > indices(BaseType::mDofSet.size());

            Element::EquationIdVectorType slave_ids(3);
            Element::EquationIdVectorType master_ids(3);
            std::unordered_map<IndexType, std::unordered_set<IndexType>> temp_indices;

            for (int i_const = 0; i_const < static_cast<int>(rModelPart.MasterSlaveConstraints().size()); ++i_const) {
                auto it_const = it_const_begin + i_const;

                // Detect if the constraint is active or not. If the user did not make any choice the constraint
                // It is active by default
                bool constraint_is_active = true;
                if( it_const->IsDefined(ACTIVE) ) {
                    constraint_is_active = it_const->Is(ACTIVE);
                }

                if(constraint_is_active) {
                    it_const->EquationIdVector(slave_ids, master_ids, r_current_process_info);

                    // here have to disable the constraint that has inactive master dof
                    // this happens because when the elements are deactivated, certain master nodes will disappear
                    // , so the corresponding constraint will be invalidated
                    for (auto &eq_id : master_ids) {
                        if (eq_id >= BaseType::mEquationSystemSize) {
                            constraint_is_active = false;
                            break;
                        }
                    }

                    if (constraint_is_active == false)
                        continue;

                    // Slave DoFs
                    for (auto &id_i : slave_ids) {
                        if (id_i < BaseType::mEquationSystemSize)
                            indices[id_i].insert(master_ids.begin(), master_ids.end());
                    }
                }
            }

            mSlaveIds.clear();
            mMasterIds.clear();
            for (int i = 0; i < static_cast<int>(indices.size()); ++i) {
                if (indices[i].size() == 0) // Master dof!
                    mMasterIds.push_back(i);
                else // Slave dof
                    mSlaveIds.push_back(i);
                indices[i].insert(i); // Ensure that the diagonal is there in T
            }

            // Count the row sizes
            std::size_t nnz = 0;
            for (IndexType i = 0; i < indices.size(); ++i)
                nnz += indices[i].size();

            mT = TSystemMatrixType(indices.size(), indices.size(), nnz);
            mConstantVector.resize(indices.size(), false);

            double *Tvalues = mT.value_data().begin();
            IndexType *Trow_indices = mT.index1_data().begin();
            IndexType *Tcol_indices = mT.index2_data().begin();

            // Filling the index1 vector - DO NOT MAKE PARALLEL THE FOLLOWING LOOP!
            Trow_indices[0] = 0;
            for (std::size_t i = 0; i < mT.size1(); ++i)
                Trow_indices[i + 1] = Trow_indices[i] + indices[i].size();

            for (std::size_t i = 0; i < mT.size1(); ++i) {
                const IndexType row_begin = Trow_indices[i];
                const IndexType row_end = Trow_indices[i + 1];
                IndexType k = row_begin;
                for (auto it = indices[i].begin(); it != indices[i].end(); ++it) {
                    Tcol_indices[k] = *it;
                    Tvalues[k] = 0.0;
                    k++;
                }

                indices[i].clear(); //deallocating the memory

                std::sort(&Tcol_indices[row_begin], &Tcol_indices[row_end]);
            }

            mT.set_filled(indices.size() + 1, nnz);

            Timer::Stop("ConstraintsRelationMatrixStructure");
        }
    }

    void BuildMasterSlaveConstraints(ModelPart& rModelPart)
    {
        KRATOS_TRY

        TSparseSpace::SetToZero(mT);
        TSparseSpace::SetToZero(mConstantVector);

        // The current process info
        const ProcessInfo& r_current_process_info = rModelPart.GetProcessInfo();

        // Vector containing the localization in the system of the different terms
        DofsVectorType slave_dof_list, master_dof_list;

        // Contributions to the system
        Matrix transformation_matrix = LocalSystemMatrixType(0, 0);
        Vector constant_vector = LocalSystemVectorType(0);

        // Vector containing the localization in the system of the different terms
        Element::EquationIdVectorType slave_equation_ids, master_equation_ids;

        const int number_of_constraints = static_cast<int>(rModelPart.MasterSlaveConstraints().size());

        // We clear the set
        mInactiveSlaveDofs.clear();

        for (int i_const = 0; i_const < number_of_constraints; ++i_const) {
            auto it_const = rModelPart.MasterSlaveConstraints().begin() + i_const;

            // Detect if the constraint is active or not. If the user did not make any choice the constraint
            // It is active by default
            bool constraint_is_active = true;
            if (it_const->IsDefined(ACTIVE))
                constraint_is_active = it_const->Is(ACTIVE);

            it_const->EquationIdVector(slave_equation_ids, master_equation_ids, r_current_process_info);

            // here have to disable the constraint that has inactive master dof
            // this happens because when the elements are deactivated, certain master nodes will disappear
            // , so the corresponding constraint will be invalidated
            for (auto &eq_id : master_equation_ids) {
                if (eq_id >= BaseType::mEquationSystemSize) {
                    constraint_is_active = false;
                    break;
                }
            }
// KRATOS_WATCH(it_const->Id())
            if (constraint_is_active) {
                it_const->CalculateLocalSystem(transformation_matrix, constant_vector, r_current_process_info);

// KRATOS_WATCH(__LINE__)
// KRATOS_WATCH(BaseType::mEquationSystemSize)
// KRATOS_WATCH(mT.size1())
// KRATOS_WATCH(mT.size2())
// KRATOS_WATCH(mConstantVector.size())
// std::cout << "slave_equation_ids:";
// for (IndexType i = 0; i < slave_equation_ids.size(); ++i)
//     std::cout << " " << slave_equation_ids[i];
// std::cout << std::endl;
// std::cout << "master_equation_ids:";
// for (IndexType i = 0; i < master_equation_ids.size(); ++i)
//     std::cout << " " << master_equation_ids[i];
// std::cout << std::endl;
                for (IndexType i = 0; i < slave_equation_ids.size(); ++i) {
                    const IndexType i_global = slave_equation_ids[i];

                    if (i_global < BaseType::mEquationSystemSize)
                    {
                        // Assemble matrix row
                        AssembleRowContribution(mT, transformation_matrix, i_global, i, master_equation_ids);

                        // Assemble constant vector
                        const double constant_value = constant_vector[i];
                        double& r_value = mConstantVector[i_global];
                        r_value += constant_value;
                    }
                }
            } else { // Taking into account inactive constraints
// KRATOS_WATCH(__LINE__)
                for (auto &eq_id : slave_equation_ids) {
                    if (eq_id < BaseType::mEquationSystemSize) {
                        mInactiveSlaveDofs.insert(eq_id);
                    }
                }
                // mInactiveSlaveDofs.insert(slave_equation_ids.begin(), slave_equation_ids.end());
// KRATOS_WATCH(__LINE__)
            }
        }

        // Setting the master dofs into the T and C system
        for (auto eq_id : mMasterIds) {
            mConstantVector[eq_id] = 0.0;
            mT(eq_id, eq_id) = 1.0;
        }

        // Setting inactive slave dofs in the T and C system
        for (auto eq_id : mInactiveSlaveDofs) {
            mConstantVector[eq_id] = 0.0;
            mT(eq_id, eq_id) = 1.0;
        }

        KRATOS_CATCH("")
    }

    void ApplyConstraints(
        typename TSchemeType::Pointer pScheme,
        TSystemMatrixType &rA,
        TSystemVectorType &rDx,
        TSystemVectorType &rb,
        ModelPart &rModelPart)
    {
        KRATOS_TRY

        if (rModelPart.MasterSlaveConstraints().size() != 0) {
            const double start_apply = OpenMPUtils::GetCurrentTime();

            BuildMasterSlaveConstraints(rModelPart);

            // We compute the transposed matrix of the global relation matrix
// KRATOS_WATCH(mT)
            TSystemMatrixType T_transpose_matrix(mT.size2(), mT.size1());
            SparseMatrixMultiplicationUtility::TransposeMatrix<TSystemMatrixType, TSystemMatrixType>(T_transpose_matrix, mT, 1.0);
// KRATOS_WATCH(T_transpose_matrix)

            TSystemVectorType b_modified(rb.size());
            TSparseSpace::Mult(T_transpose_matrix, rb, b_modified);
            TSparseSpace::Copy(b_modified, rb);
            b_modified.resize(0, false); //free memory

            TSystemMatrixType auxiliar_A_matrix(mT.size2(), rA.size2());
            SparseMatrixMultiplicationUtility::MatrixMultiplication(T_transpose_matrix, rA, auxiliar_A_matrix); //auxiliar = T_transpose * rA
            T_transpose_matrix.resize(0, 0, false);                                                             //free memory

            SparseMatrixMultiplicationUtility::MatrixMultiplication(auxiliar_A_matrix, mT, rA); //A = auxilar * T   NOTE: here we are overwriting the old A matrix!
            auxiliar_A_matrix.resize(0, 0, false);                                              //free memory

            double max_diag = 0.0;
            for(IndexType i = 0; i < rA.size1(); ++i) {
                max_diag = std::max(std::abs(rA(i,i)), max_diag);
            }

            // Apply diagonal values on slaves
            // #pragma omp parallel for
            for (int i = 0; i < static_cast<int>(mSlaveIds.size()); ++i) {
                const IndexType slave_equation_id = mSlaveIds[i];
                if (mInactiveSlaveDofs.find(slave_equation_id) == mInactiveSlaveDofs.end()) {
                    if (slave_equation_id < BaseType::mEquationSystemSize) {
                        rA(slave_equation_id, slave_equation_id) = max_diag;
                        rb[slave_equation_id] = 0.0;
                    }
                }
            }

            const double stop_apply = OpenMPUtils::GetCurrentTime();

            if (this->GetEchoLevel() >= 1 && rModelPart.GetCommunicator().MyPID() == 0)
            {
                std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Apply Constraints time: " << stop_apply - start_apply << std::endl;
            }
        }

        KRATOS_CATCH("")
    }

    virtual void ConstructMatrixStructure(
        TSystemMatrixType& A,
        ModelPart& rModelPart)
    {
        if ( this->GetEchoLevel() > 1 && rModelPart.GetCommunicator().MyPID() == 0)
        {
            std::cout << "ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation: " << "Entering ConstructMatrixStructure" << std::endl;
        }

        //filling with zero the matrix (creating the structure)
        Timer::Start("MatrixStructure");

        ProcessInfo& CurrentProcessInfo = rModelPart.GetProcessInfo();
        ElementsArrayType& r_elements_array = rModelPart.Elements();
        ConditionsArrayType& r_conditions_array = rModelPart.Conditions();

        const std::size_t equation_size = BaseType::mEquationSystemSize;

        std::vector<std::unordered_set<std::size_t> > indices(equation_size);
        for (std::size_t iii = 0; iii < BaseType::mEquationSystemSize; iii++)
        {
            indices[iii].reserve(40);
        }

        Element::EquationIdVectorType ids(3, 0);

        bool element_is_active;
        for(typename ElementsArrayType::ptr_iterator it = r_elements_array.ptr_begin();
                it != r_elements_array.ptr_end(); ++it)
        {
            element_is_active = true;
            if( (*it)->IsDefined(ACTIVE) ) {
                element_is_active = (*it)->Is(ACTIVE);
            }

            if (!element_is_active)
                continue;

            (*it)->EquationIdVector(ids, CurrentProcessInfo);
            for (std::size_t i = 0; i < ids.size(); i++) {
                if (ids[i] < BaseType::mEquationSystemSize)
                {
                    auto& row_indices = indices[ids[i]];
                    row_indices.insert(ids.begin(), ids.end());
                }
            }
        }

        bool condition_is_active;
        for(typename ConditionsArrayType::ptr_iterator it = r_conditions_array.ptr_begin();
                it != r_conditions_array.ptr_end(); ++it)
        {
            condition_is_active = true;
            if( (*it)->IsDefined(ACTIVE) ) {
                condition_is_active = (*it)->Is(ACTIVE);
            }

            if (!condition_is_active)
                continue;

            (*it)->EquationIdVector(ids, CurrentProcessInfo);
            for (std::size_t i = 0; i < ids.size(); i++) {
                if (ids[i] < BaseType::mEquationSystemSize)
                {
                    auto& row_indices = indices[ids[i]];
                    row_indices.insert(ids.begin(), ids.end());
                }
            }
        }

        unsigned int nnz = 0;
        for (unsigned int i = 0; i < indices.size(); i++) {
            nnz += indices[i].size();
        }

        A = CompressedMatrixType(indices.size(), indices.size(), nnz);

        double* Avalues = A.value_data().begin();
        std::size_t* Arow_indices = A.index1_data().begin();
        std::size_t* Acol_indices = A.index2_data().begin();

        //filling the index1 vector - DO NOT MAKE PARALLEL THE FOLLOWING LOOP!
        Arow_indices[0] = 0;
        for (std::size_t i = 0; i < A.size1(); i++)
        {
            Arow_indices[i+1] = Arow_indices[i] + indices[i].size();
        }

        for (std::size_t i = 0; i < A.size1(); i++)
        {
            const unsigned int row_begin = Arow_indices[i];
            const unsigned int row_end = Arow_indices[i+1];
            unsigned int k = row_begin;
            for (auto it = indices[i].begin(); it != indices[i].end(); it++) {
                Acol_indices[k] = *it;
                Avalues[k] = 0.0;
                k++;
            }

            indices[i].clear(); //deallocating the memory

            std::sort(&Acol_indices[row_begin], &Acol_indices[row_end]);
        }

        A.set_filled(indices.size()+1, nnz);

        Timer::Stop("MatrixStructure");
    }

    void Assemble(
        TSystemMatrixType& A,
        TSystemVectorType& b,
        const LocalSystemMatrixType& LHS_Contribution,
        const LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId
    )
    {
        unsigned int local_size = LHS_Contribution.size1();

        for (unsigned int i_local = 0; i_local < local_size; i_local++) {
            unsigned int i_global = EquationId[i_local];

            double& r_a = b[i_global];
            const double& v_a = RHS_Contribution(i_local);
            #pragma omp atomic
            r_a += v_a;

            AssembleRowContribution(A, LHS_Contribution, i_global, i_local, EquationId);
        }
    }


    //**************************************************************************

    void AssembleRHS(
        TSystemVectorType& b,
        LocalSystemVectorType& RHS_Contribution,
        Element::EquationIdVectorType& EquationId
    )
    {
        unsigned int local_size = RHS_Contribution.size();

        for (unsigned int i_local = 0; i_local < local_size; i_local++) {
            unsigned int i_global = EquationId[i_local];

            // ASSEMBLING THE SYSTEM VECTOR
            double& b_value = b[i_global];
            const double& rhs_value = RHS_Contribution[i_local];

            #pragma omp atomic
            b_value += rhs_value;
        }
    }

    ///@}
    ///@name Protected  Access
    ///@{

    ///@}
    ///@name Protected Inquiry
    ///@{

    ///@}
    ///@name Protected LifeCycle
    ///@{

    ///@}

private:
    ///@name Static Member Variables
    ///@{

    ///@}
    ///@name Member Variables
    ///@{

    DofsArrayType mAllDofs;

    ///@}
    ///@name Private Operators
    ///@{

    ///@}
    ///@name Private Operations
    ///@{

    inline void AddUnique(std::vector<std::size_t>& v, const std::size_t& candidate)
    {
        std::vector<std::size_t>::iterator i = v.begin();
        std::vector<std::size_t>::iterator endit = v.end();
        while (i != endit && (*i) != candidate) {
            i++;
        }
        if (i == endit) {
            v.push_back(candidate);
        }

    }

    void BuildRHSNoDirichlet(
        typename TSchemeType::Pointer pScheme,
        ModelPart& rModelPart,
        TSystemVectorType& b)
    {
        KRATOS_TRY

        //Getting the Elements
        ElementsArrayType& pElements = rModelPart.Elements();

        //getting the array of the conditions
        ConditionsArrayType& ConditionsArray = rModelPart.Conditions();

        ProcessInfo& CurrentProcessInfo = rModelPart.GetProcessInfo();

        //contributions to the system
        LocalSystemMatrixType LHS_Contribution = LocalSystemMatrixType(0, 0);
        LocalSystemVectorType RHS_Contribution = LocalSystemVectorType(0);

        //vector containing the localization in the system of the different
        //terms
        Element::EquationIdVectorType EquationId;

        // assemble all elements
        //for (typename ElementsArrayType::ptr_iterator it = pElements.ptr_begin(); it != pElements.ptr_end(); ++it)

        const int nelements = static_cast<int>(pElements.size());
        #pragma omp parallel firstprivate(nelements, RHS_Contribution, EquationId)
        {
            #pragma omp for schedule(guided, 512) nowait
            for (int i=0; i<nelements; i++) {
                typename ElementsArrayType::iterator it = pElements.begin() + i;
                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool element_is_active = true;
                if( (it)->IsDefined(ACTIVE) ) {
                    element_is_active = (it)->Is(ACTIVE);
                }

                if(element_is_active) {
                    //calculate elemental Right Hand Side Contribution
                    pScheme->Calculate_RHS_Contribution(*(it.base()), RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution
                    AssembleRHS(b, RHS_Contribution, EquationId);
                }
            }

            LHS_Contribution.resize(0, 0, false);
            RHS_Contribution.resize(0, false);

            // assemble all conditions
            const int nconditions = static_cast<int>(ConditionsArray.size());
            #pragma omp for schedule(guided, 512)
            for (int i = 0; i<nconditions; i++) {
                auto it = ConditionsArray.begin() + i;
                //detect if the element is active or not. If the user did not make any choice the element
                //is active by default
                bool condition_is_active = true;
                if( (it)->IsDefined(ACTIVE) ) {
                    condition_is_active = (it)->Is(ACTIVE);
                }

                if(condition_is_active) {
                    //calculate elemental contribution
                    pScheme->Condition_Calculate_RHS_Contribution(*(it.base()), RHS_Contribution, EquationId, CurrentProcessInfo);

                    //assemble the elemental contribution
                    AssembleRHS(b, RHS_Contribution, EquationId);
                }
            }
        }

        KRATOS_CATCH("")

    }

    //******************************************************************************************
    //******************************************************************************************

    inline void CreatePartition(unsigned int number_of_threads, const int number_of_rows, vector<unsigned int>& partitions)
    {
        partitions.resize(number_of_threads + 1);
        int partition_size = number_of_rows / number_of_threads;
        partitions[0] = 0;
        partitions[number_of_threads] = number_of_rows;
        for (unsigned int i = 1; i < number_of_threads; i++) {
            partitions[i] = partitions[i - 1] + partition_size;
        }
    }

    inline void AssembleRowContribution(TSystemMatrixType& A, const Matrix& Alocal, const unsigned int i, const unsigned int i_local, Element::EquationIdVectorType& EquationId)
    {
        double* values_vector = A.value_data().begin();
        std::size_t* index1_vector = A.index1_data().begin();
        std::size_t* index2_vector = A.index2_data().begin();

        size_t left_limit = index1_vector[i];
//    size_t right_limit = index1_vector[i+1];

        //find the first entry
        size_t last_pos = ForwardFind(EquationId[0],left_limit,index2_vector);
        size_t last_found = EquationId[0];

        double& r_a = values_vector[last_pos];
        const double& v_a = Alocal(i_local,0);
        #pragma omp atomic
        r_a +=  v_a;

        //now find all of the other entries
        size_t pos = 0;
        for (unsigned int j=1; j<EquationId.size(); j++) {
            unsigned int id_to_find = EquationId[j];
            if(id_to_find > last_found) {
                pos = ForwardFind(id_to_find,last_pos+1,index2_vector);
            } else if(id_to_find < last_found) {
                pos = BackwardFind(id_to_find,last_pos-1,index2_vector);
            } else {
                pos = last_pos;
            }

            double& r = values_vector[pos];
            const double& v = Alocal(i_local,j);
            #pragma omp atomic
            r +=  v;

            last_found = id_to_find;
            last_pos = pos;
        }
    }

    inline unsigned int ForwardFind(const unsigned int id_to_find,
                                    const unsigned int start,
                                    const size_t* index_vector)
    {
        unsigned int pos = start;
        while(id_to_find != index_vector[pos]) pos++;
        return pos;
    }

    inline unsigned int BackwardFind(const unsigned int id_to_find,
                                     const unsigned int start,
                                     const size_t* index_vector)
    {
        unsigned int pos = start;
        while(id_to_find != index_vector[pos]) pos--;
        return pos;
    }

    ///@}
    ///@name Private Operations
    ///@{

    ///@}
    ///@name Private  Access
    ///@{

    ///@}
    ///@name Private Inquiry
    ///@{

    ///@}
    ///@name Un accessible methods
    ///@{

    ///@}

}; /* Class ResidualBasedBlockBuilderAndSolverWithConstraintsDeactivation */

///@}

///@name Type Definitions
///@{


///@}

} /* namespace Kratos.*/

#endif /* KRATOS_RESIDUAL_BASED_BLOCK_BUILDER_AND_SOLVER_WITH_CONSTRAINTS_DEACTIVATION  defined */

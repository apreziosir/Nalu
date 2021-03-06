/*------------------------------------------------------------------------*/
/*  Copyright 2014 Sandia Corporation.                                    */
/*  This software is released under the license detailed                  */
/*  in the file, LICENSE, which is located in the top-level Nalu          */
/*  directory structure                                                   */
/*------------------------------------------------------------------------*/


#include <SolutionOptions.h>
#include <Enums.h>
#include <NaluEnv.h>

// basic c++
#include <stdexcept>
#include <utility>

namespace sierra{
namespace nalu{

//==========================================================================
// Class Definition
//==========================================================================
// SolutionOptions - holder for user options at the realm scope
//==========================================================================
//--------------------------------------------------------------------------
//-------- constructor -----------------------------------------------------
//--------------------------------------------------------------------------
SolutionOptions::SolutionOptions()
  : hybridDefault_(0.0),
    alphaDefault_(0.0),
    alphaUpwDefault_(1.0),
    upwDefault_(1.0),
    lamScDefault_(1.0),
    turbScDefault_(1.0),
    turbPrDefault_(1.0),
    nocDefault_(true),
    pecletFunctionalFormDefault_("classic"),
    pecletTanhTransDefault_(2.0),
    pecletTanhWidthDefault_(4.0),
    referenceDensity_(0.0),
    referenceTemperature_(298.0),
    thermalExpansionCoeff_(1.0),
    stefanBoltzmann_(5.6704e-8),
    nearestFaceEntrain_(0.0),
    includeDivU_(0.0),
    mdotInterpRhoUTogether_(true),
    isTurbulent_(false),
    turbulenceModel_(LAMINAR),
    meshMotion_(false),
    meshDeformation_(false),
    externalMeshDeformation_(false),
    activateUniformRefinement_(false),
    uniformRefineSaveAfter_(false),
    activateAdaptivity_(false),
    errorIndicatorType_(EIT_NONE),
    adaptivityFrequency_(0),
    useMarker_(false),
    refineFraction_(0.0),
    unrefineFraction_(0.0),
    physicalErrIndCriterion_(0.0),
    physicalErrIndUnrefCriterionMultipler_(1.0),
    maxRefinementNumberOfElementsFraction_(0),
    adapterExtraOutput_(false),
    useAdapter_(false),
    maxRefinementLevel_(0),
    extrusionCorrectionFac_(1.0),
    ncAlgType_(NC_ALG_TYPE_DG),
    ncAlgGaussLabatto_(true),
    ncAlgUpwindAdvection_(false),
    ncAlgIncludePstab_(true),
    ncAlgDetailedOutput_(false),
    ncAlgCurrentNormal_(false),
    cvfemShiftMdot_(false),
    cvfemShiftPoisson_(false),
    cvfemReducedSensPoisson_(false),
    inputVariablesRestorationTime_(1.0e8),
    inputVariablesInterpolateInTime_(false),
    consistentMMPngDefault_(false),
    useConsolidatedSolverAlg_(false),
    eigenvaluePerturb_(false),
    eigenvaluePerturbDelta_(0.0),
    eigenvaluePerturbBiasTowards_(3),
    eigenvaluePerturbTurbKe_(0.0)
{
  // nothing to do
}

//--------------------------------------------------------------------------
//-------- destructor ------------------------------------------------------
//--------------------------------------------------------------------------
SolutionOptions::~SolutionOptions()
{
  // nothing to do
}

//--------------------------------------------------------------------------
//-------- load ------------------------------------------------------------
//--------------------------------------------------------------------------
void
SolutionOptions::load(const YAML::Node & y_node)
{
  const bool optional=true;
  const bool required=!optional;

  const YAML::Node *y_solution_options = expect_map(y_node,"solution_options", optional);
  if(y_solution_options)
  {
    get_required(*y_solution_options, "name", name_);
    get_if_present(*y_solution_options,
                   "nearest_face_entrainment",
                   nearestFaceEntrain_, nearestFaceEntrain_);

    // divU factor for stress
    get_if_present(*y_solution_options, "divU_stress_scaling", includeDivU_, includeDivU_);

    // mdot interpolation procedure 
    get_if_present(*y_solution_options, "interp_rhou_together_for_mdot", mdotInterpRhoUTogether_, mdotInterpRhoUTogether_);

    // extrustion correction scaling
    get_if_present(*y_solution_options, 
                   "extrusion_correction_factor", 
                   extrusionCorrectionFac_, extrusionCorrectionFac_);
    
    // external mesh motion expected
    get_if_present(*y_solution_options, "externally_provided_mesh_deformation", externalMeshDeformation_, externalMeshDeformation_);

    // shifted CVFEM pressure poisson
    get_if_present(*y_solution_options, "shift_cvfem_mdot", cvfemShiftMdot_, cvfemShiftMdot_);
    get_if_present(*y_solution_options, "shift_cvfem_poisson", cvfemShiftPoisson_, cvfemShiftPoisson_);
    get_if_present(*y_solution_options, "reduced_sens_cvfem_poisson", cvfemReducedSensPoisson_, cvfemReducedSensPoisson_);
    if ( cvfemShiftMdot_ )
      NaluEnv::self().naluOutputP0() << "Shifted CVFEM mass flow rate" << std::endl;
    if ( cvfemShiftPoisson_ )
      NaluEnv::self().naluOutputP0() << "Shifted CVFEM Poisson" << std::endl;
    if ( cvfemReducedSensPoisson_)
      NaluEnv::self().naluOutputP0() << "Reduced sensitivities CVFEM Poisson" << std::endl;

    // sanity checks; if user asked for shifted Poisson, then user will have reduced sensitivities
    if ( cvfemShiftPoisson_ ) {
      if ( !cvfemReducedSensPoisson_) {
        NaluEnv::self().naluOutputP0() << "Reduced sensitivities CVFEM Poisson will be set" << std::endl;
        cvfemReducedSensPoisson_ = true;
      }
    }

    // check for consolidated solver alg (AssembleSolver)
    get_if_present(*y_solution_options, "use_consolidated_solver_algorithm", useConsolidatedSolverAlg_, useConsolidatedSolverAlg_);

    // eigenvalue purturbation; over all dofs...
    get_if_present(*y_solution_options, "eigenvalue_perturbation", eigenvaluePerturb_);
    get_if_present(*y_solution_options, "eigenvalue_perturbation_delta", eigenvaluePerturbDelta_);
    get_if_present(*y_solution_options, "eigenvalue_perturbation_bias_towards", eigenvaluePerturbBiasTowards_);
    get_if_present(*y_solution_options, "eigenvalue_perturbation_turbulent_ke", eigenvaluePerturbTurbKe_);
    
    // extract turbulence model; would be nice if we could parse an enum..
    std::string specifiedTurbModel;
    std::string defaultTurbModel = "laminar";
    get_if_present(*y_solution_options,
        "turbulence_model", specifiedTurbModel, defaultTurbModel);
    for ( int k=0; k < TurbulenceModel_END; ++k ) {
      if ( specifiedTurbModel == TurbulenceModelNames[k] ) {
        turbulenceModel_ = TurbulenceModel(k);
        break;
      }
    }
    if ( turbulenceModel_ != LAMINAR ) {
      isTurbulent_ = true;
    }
    // initialize turbuelnce constants since some laminar models may need such variables, e.g., kappa
    initialize_turbulence_constants();

    // extract possible copy from input fields restoration time
    get_if_present(*y_solution_options, "input_variables_from_file_restoration_time",
      inputVariablesRestorationTime_, inputVariablesRestorationTime_);

    // choice of interpolation or snapping to closest in the data base
    get_if_present(*y_solution_options, "input_variables_interpolate_in_time",
      inputVariablesInterpolateInTime_, inputVariablesInterpolateInTime_);

    // first set of options; hybrid, source, etc.
    const YAML::Node *y_options = expect_sequence(*y_solution_options, "options", required);
    if (y_options)
    {
      for (size_t ioption = 0; ioption < y_options->size(); ++ioption)
      {
        const YAML::Node & y_option = (*y_options)[ioption];
        if (expect_map(y_option, "hybrid_factor", optional)) {
          y_option["hybrid_factor"] >> hybridMap_;
        }
        else if (expect_map(y_option, "alpha", optional)) {
          y_option["alpha"] >> alphaMap_;
        }
        else if (expect_map(y_option, "alpha_upw", optional)) {
          y_option["alpha_upw"] >> alphaUpwMap_;
        }
        else if (expect_map(y_option, "upw_factor", optional)) {
          y_option["upw_factor"] >> upwMap_;
        }
        else if (expect_map(y_option, "limiter", optional)) {
          y_option["limiter"] >> limiterMap_;
        }
        else if (expect_map( y_option, "laminar_schmidt", optional)) {
          y_option["laminar_schmidt"] >> lamScMap_;
        }
        else if (expect_map( y_option, "laminar_prandtl", optional)) {
          y_option["laminar_prandtl"] >> lamPrMap_;
        }
        else if (expect_map( y_option, "turbulent_schmidt", optional)) {
          y_option["turbulent_schmidt"] >> turbScMap_;
        }
        else if (expect_map( y_option, "turbulent_prandtl", optional)) {
          y_option["turbulent_prandtl"] >> turbPrMap_;
        }
        else if (expect_map( y_option, "source_terms", optional)) {
          const YAML::Node& ySrc = *y_option.FindValue("source_terms");
          ySrc >> srcTermsMap_;
        }
        else if (expect_map( y_option, "element_source_terms", optional)) {
          const YAML::Node& ySrc = *y_option.FindValue("element_source_terms");
          ySrc >> elemSrcTermsMap_;
        }
        else if (expect_map( y_option, "source_term_parameters", optional)) {
            y_option["source_term_parameters"] >> srcTermParamMap_;
        }
        else if (expect_map( y_option, "element_source_term_parameters", optional)) {
          y_option["element_source_term_parameters"] >> elemSrcTermParamMap_;
        }
        else if (expect_map( y_option, "projected_nodal_gradient", optional)) {
          y_option["projected_nodal_gradient"] >> nodalGradMap_;
        }
        else if (expect_map( y_option, "noc_correction", optional)) {
          y_option["noc_correction"] >> nocMap_;
        }
        else if (expect_map( y_option, "input_variables_from_file", optional)) {
          y_option["input_variables_from_file"] >> inputVarFromFileMap_;
        }
        else if (expect_map( y_option, "turbulence_model_constants", optional)) {
          std::map<std::string, double> turbConstMap;
          y_option["turbulence_model_constants"] >> turbConstMap;
          // iterate the parsed map
          std::map<std::string, double>::iterator it;
          for ( it = turbConstMap.begin(); it!= turbConstMap.end(); ++it ) {
            std::string theConstName = it->first;
            double theConstValue = it->second;
            // find the enum and set the value
            bool foundIt = false;
            for ( int k=0; k < TM_END; ++k ) {
              if ( theConstName == TurbulenceModelConstantNames[k] ) {
                TurbulenceModelConstant theConstEnum = TurbulenceModelConstant(k);
                turbModelConstantMap_[theConstEnum] = theConstValue;
                foundIt = true;
                break;
              }
            }
            // error check..
            if ( !foundIt ) {
              NaluEnv::self().naluOutputP0() << "Sorry, turbulence model constant with name " << theConstName << " was not found " << std::endl;
              NaluEnv::self().naluOutputP0() << "List of turbulence model constant names are as follows:" << std::endl;
              for ( int k=0; k < TM_END; ++k ) {
                NaluEnv::self().naluOutputP0() << TurbulenceModelConstantNames[k] << std::endl;
              }
            }
          }
        }
        else if (expect_map( y_option, "user_constants", optional)) {
          const YAML::Node& y_user_constants = *y_option.FindValue("user_constants");
          get_if_present(y_user_constants, "reference_density",  referenceDensity_, referenceDensity_);
          get_if_present(y_user_constants, "reference_temperature",  referenceTemperature_, referenceTemperature_);
          get_if_present(y_user_constants, "thermal_expansion_coefficient",  thermalExpansionCoeff_, thermalExpansionCoeff_);
          get_if_present(y_user_constants, "stefan_boltzmann",  stefanBoltzmann_, stefanBoltzmann_);
          if (expect_sequence( y_user_constants, "gravity", optional) ) {
            const int gravSize = y_user_constants["gravity"].size();
            gravity_.resize(gravSize);
            for (int i = 0; i < gravSize; ++i ) {
              y_user_constants["gravity"][i] >> gravity_[i];
            }
          }
        }
        else if (expect_map( y_option, "non_conformal", optional)) {
          const YAML::Node& y_nc = *y_option.FindValue("non_conformal");
          get_if_present(y_nc, "gauss_labatto_quadrature",  ncAlgGaussLabatto_, ncAlgGaussLabatto_);
          get_if_present(y_nc, "upwind_advection",  ncAlgUpwindAdvection_, ncAlgUpwindAdvection_);
          get_if_present(y_nc, "include_pstab",  ncAlgIncludePstab_, ncAlgIncludePstab_);
          get_if_present(y_nc, "detailed_output",  ncAlgDetailedOutput_, ncAlgDetailedOutput_);
          get_if_present(y_nc, "current_normal",  ncAlgCurrentNormal_, ncAlgCurrentNormal_);
          if (y_nc.FindValue("algorithm_type" )  ) {
            std::string algTypeString = "none";
            y_nc["algorithm_type"] >> algTypeString;
            // find the enum and set the value
            bool foundIt = false;
            for ( int k=0; k < NC_ALG_TYPE_END; ++k ) {
              if ( algTypeString == NonConformalAlgTypeNames[k] ) {
                NonConformalAlgType algTypeEnum = NonConformalAlgType(k);
                foundIt = true;
                ncAlgType_ = algTypeEnum;
                break;
              }
            }
            if ( !foundIt )
              NaluEnv::self().naluOutputP0() << "Cound not find: " << algTypeString << std::endl;
          }
        }
        else if (expect_map( y_option, "peclet_function_form", optional)) {
          y_option["peclet_function_form"] >> pecletFunctionalFormMap_;
        }
        else if (expect_map( y_option, "peclet_function_tanh_transition", optional)) {
          y_option["peclet_function_tanh_transition"] >> pecletFunctionTanhTransMap_;
        }
        else if (expect_map( y_option, "peclet_function_tanh_width", optional)) {
          y_option["peclet_function_tanh_width"] >> pecletFunctionTanhWidthMap_;
        }
        else if (expect_map( y_option, "consistent_mass_matrix_png", optional)) {
          y_option["consistent_mass_matrix_png"] >> consistentMassMatrixPngMap_;
        }
        else {
          if (!NaluEnv::self().parallel_rank())
          {
            std::cout << "Error: parsing at " << NaluParsingHelper::info(y_option)
              //<< "... at parent ... " << NaluParsingHelper::info(y_node)
                      << std::endl;
          }
          throw std::runtime_error("unknown solution option: "+ NaluParsingHelper::info(y_option));
        }
      }
    }

    // second set of options: mesh motion... this means that the Realm will expect to provide rotation-based motion
    const YAML::Node *y_mesh_motion = expect_sequence(*y_solution_options, "mesh_motion", optional);
    if (y_mesh_motion)
    {
      // mesh motion is active
      meshMotion_ = true;

      // has a user stated that mesh motion is external?
      if ( meshDeformation_ ) {
        NaluEnv::self().naluOutputP0() << "mesh motion set to external (will prevail over mesh motion specification)!" << std::endl;
      }
      else {        
        for (size_t ioption = 0; ioption < y_mesh_motion->size(); ++ioption) {
          const YAML::Node &y_option = (*y_mesh_motion)[ioption];
          
          // extract mesh motion name and omega value
          std::string motionName = "na";
          get_required(y_option, "name", motionName);
          double omega = 0.0;
          get_required(y_option, "omega", omega);
          
          // now fill in name
          std::vector<std::string> meshMotionBlock;
          const YAML::Node &targets = y_option["target_name"];
          if (targets.Type() == YAML::NodeType::Scalar) {
            meshMotionBlock.resize(1);
            targets >> meshMotionBlock[0];
          }
          else {
            meshMotionBlock.resize(targets.size());
            for (size_t i=0; i < targets.size(); ++i) {
              targets[i] >> meshMotionBlock[i];
            }
          }
          std::pair<std::vector<std::string>, double > thePair;
          thePair = std::make_pair(meshMotionBlock, omega);
          
          // provide the map
          meshMotionMap_[motionName] = thePair;
          
          // check for centroids; provide default
          Coordinates cCoords;
          const YAML::Node *coordsNode = y_option.FindValue("centroid_coordinates");
          if ( coordsNode )
            *coordsNode >> cCoords;
          meshMotionCentroidMap_[motionName] = cCoords;
        }
      }
    }

    // uniform refinement options
    {
      const YAML::Node *y_uniform = expect_map(*y_solution_options, "uniform_refinement", optional);
      if (y_uniform) {

        NaluEnv::self().naluOutputP0() << "Uniform refinement option found." << std::endl;

        const YAML::Node *y_refine_at = expect_sequence(*y_uniform, "refine_at", required);
        if (y_refine_at) {
          activateUniformRefinement_ = true;
          std::vector<int> mvec;
          *y_refine_at >> mvec;
          for (unsigned i=0; i < mvec.size(); ++i) {
            NaluEnv::self().naluOutputP0() << "Uniform Refinement: refine_at[" << i << "]= " << mvec[i] << std::endl;

            if (i > 0 && mvec[i-1] > mvec[i])
              throw std::runtime_error("refine_at option error: "+ NaluParsingHelper::info(*y_refine_at));
          }
          refineAt_ = mvec;
        }
        else {
          throw std::runtime_error("refine_at option missing: "+ NaluParsingHelper::info(*y_uniform));
        }
        get_if_present(*y_uniform, "save_mesh", uniformRefineSaveAfter_, uniformRefineSaveAfter_);
        NaluEnv::self().naluOutputP0() << "Uniform Refinement: save_mesh= " << uniformRefineSaveAfter_ << std::endl;
      }
    }

    // adaptivity options
    const YAML::Node *y_adaptivity = expect_map(*y_solution_options, "adaptivity", optional);
    if (y_adaptivity) {

#if defined (NALU_USES_PERCEPT)
      NaluEnv::self().naluOutputP0() << "Adaptivity Active. tested on Tri, Tet and quad meshes " << std::endl;
#else
      throw std::runtime_error("Adaptivity not supported in NaluV1.0:");
#endif
      
      get_if_present(*y_adaptivity, "frequency", adaptivityFrequency_, adaptivityFrequency_);
      get_if_present(*y_adaptivity, "activate", activateAdaptivity_, activateAdaptivity_);

      if (activateAdaptivity_ && adaptivityFrequency_<1) {
	throw std::runtime_error("When adaptivity is active, the frequency must by greater than 0:" + NaluParsingHelper::info(*y_adaptivity));
      }

      const YAML::Node *y_error_indicator = expect_map(*y_adaptivity, "error_indicator", required);
      if (y_error_indicator)
      {
        std::string type = "";
        get_if_present(*y_error_indicator, "type", type, type);
        if (type == "pstab")
          errorIndicatorType_ = EIT_PSTAB;
        else if (type == "limiter")
          errorIndicatorType_ = EIT_LIMITER;

        // error catching and user output
        if ( errorIndicatorType_ == EIT_NONE ) {
          NaluEnv::self().naluOutputP0() << "no or unknown error indicator was provided; will choose pstab.  Input value= " << type << std::endl;
          errorIndicatorType_ = EIT_PSTAB;
        }

        // for debugging/testing use only
        if (type == "simple.vorticity_dx")
          errorIndicatorType_ = EIT_SIMPLE_VORTICITY_DX;
        else if (type == "simple.vorticity")
          errorIndicatorType_ = EIT_SIMPLE_VORTICITY;
        else if (type == "simple.dudx2")
          errorIndicatorType_ = EIT_SIMPLE_DUDX2;
        if (errorIndicatorType_ & EIT_SIMPLE_BASE) {
          NaluEnv::self().naluOutputP0() << "WARNING: Found debug/test error inidicator type. Input value= " << type << std::endl;
        }
      }

      NaluEnv::self().naluOutputP0() << std::endl;
      NaluEnv::self().naluOutputP0() << "Adaptivity Options Review: " << std::endl;
      NaluEnv::self().naluOutputP0() << "===========================" << std::endl;
      NaluEnv::self().naluOutputP0() << " pstab: " << (errorIndicatorType_ & EIT_PSTAB)
                      << " limit: " << (errorIndicatorType_ & EIT_LIMITER)
                      << " freq : " << adaptivityFrequency_ << std::endl;

      const YAML::Node *y_adapter = expect_map(*y_adaptivity, "adapter", optional);
      bool marker_optional = true;
      if (y_adapter) {
        get_if_present(*y_adapter, "activate", useAdapter_, useAdapter_);
        get_if_present(*y_adapter, "max_refinement_level", maxRefinementLevel_, maxRefinementLevel_);
        get_if_present(*y_adapter, "extra_output", adapterExtraOutput_, adapterExtraOutput_);
        marker_optional = false;
      }

      const YAML::Node *y_marker = expect_map(*y_adaptivity, "marker", marker_optional);
      if (y_marker) {
        bool defaultUseMarker = true;
        get_if_present(*y_marker, "activate", useMarker_, defaultUseMarker);
        get_if_present(*y_marker, "refine_fraction", refineFraction_, refineFraction_);
        get_if_present(*y_marker, "unrefine_fraction", unrefineFraction_, unrefineFraction_);
        get_if_present(*y_marker, "max_number_elements_fraction", maxRefinementNumberOfElementsFraction_, maxRefinementNumberOfElementsFraction_);
        get_if_present(*y_marker, "physical_error_criterion", physicalErrIndCriterion_, physicalErrIndCriterion_);
        get_if_present(*y_marker, "physical_error_criterion_unrefine_multiplier", physicalErrIndUnrefCriterionMultipler_, physicalErrIndUnrefCriterionMultipler_);
      }


#define OUTN(a) " " << #a << " = " << a

      NaluEnv::self().naluOutputP0() << "Adapt: options: "
                      << OUTN(activateAdaptivity_)
                      << OUTN(errorIndicatorType_)
                      << OUTN(adaptivityFrequency_) << "\n"
                      << OUTN(useMarker_)
                      << OUTN(refineFraction_)
                      << OUTN(unrefineFraction_)
                      << OUTN(physicalErrIndCriterion_)
                      << OUTN(physicalErrIndUnrefCriterionMultipler_)
                      << OUTN(maxRefinementNumberOfElementsFraction_) << "\n"
                      << OUTN(useAdapter_)
                      << OUTN(maxRefinementLevel_)
                      << std::endl;

    }
  }

   NaluEnv::self().naluOutputP0() << std::endl;
   NaluEnv::self().naluOutputP0() << "Turbulence Model Review:   " << std::endl;
   NaluEnv::self().naluOutputP0() << "===========================" << std::endl;
   NaluEnv::self().naluOutputP0() << "Turbulence Model is: "
       << TurbulenceModelNames[turbulenceModel_] << " " << isTurbulent_ <<std::endl;

}

//--------------------------------------------------------------------------
//-------- initialize_turbulence_constants ---------------------------------
//--------------------------------------------------------------------------
void
SolutionOptions::initialize_turbulence_constants() 
{
  // set the default map values; resize to max turbulence model enum
  turbModelConstantMap_[TM_cMu] = 0.09; 
  turbModelConstantMap_[TM_kappa] = 0.41;
  turbModelConstantMap_[TM_cDESke] = 0.61; 
  turbModelConstantMap_[TM_cDESkw] = 0.78;
  turbModelConstantMap_[TM_tkeProdLimitRatio] = (turbulenceModel_ == SST || turbulenceModel_ == SST_DES) ? 10.0 : 500.0;
  turbModelConstantMap_[TM_cmuEps] = 0.0856; 
  turbModelConstantMap_[TM_cEps] = 0.845;
  turbModelConstantMap_[TM_betaStar] = 0.09;
  turbModelConstantMap_[TM_aOne] = 0.31;
  turbModelConstantMap_[TM_betaOne] = 0.075;
  turbModelConstantMap_[TM_betaTwo] = 0.0828;
  turbModelConstantMap_[TM_gammaOne] = 5.0/9.0;
  turbModelConstantMap_[TM_gammaTwo] = 0.44;
  turbModelConstantMap_[TM_sigmaKOne] = 0.85;
  turbModelConstantMap_[TM_sigmaKTwo] = 1.0;
  turbModelConstantMap_[TM_sigmaWOne] = 0.50;
  turbModelConstantMap_[TM_sigmaWTwo] = 0.856;
  turbModelConstantMap_[TM_cmuCs] = 0.17;
  turbModelConstantMap_[TM_Cw] = 0.325;
  turbModelConstantMap_[TM_CbTwo] = 0.35;
}

} // namespace nalu
} // namespace Sierra

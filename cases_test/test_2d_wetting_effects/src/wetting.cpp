/**
 * @file 	two_phase_dambreak.cpp
 * @brief 	2D two-phase dambreak flow.
 * @details This is the one of the basic test cases, also the first case for
 * 			understanding SPH method for multi-phase simulation.
 * @author 	Chi Zhang and Xiangyu Hu
 */
#include "case.h"
#include "sphinxsys.h"
using namespace SPH;
/**
 * @brief 	Main program starts here.
 */
int main()
{
	/**
	 * @brief Build up -- a SPHSystem --
	 */
	SPHSystem sph_system(system_domain_bounds, particle_spacing_ref);
	/** Set the starting time. */
	GlobalStaticVariables::physical_time_ = 0.0;
	/** Tag for computation from restart files. 0: not from restart files. */
	sph_system.restart_step_ = 0;
	/**
	 * @brief Material property, partilces and body creation of water.
	 */
	WaterBlock* water_block = new WaterBlock(sph_system, "WaterBody");
	WaterMaterial* water_material = new WaterMaterial();
	FluidParticles 	water_particles(water_block, water_material);
	/**
	 * @brief Material property, partilces and body creation of air.
	 */
	AirBlock* air_block = new AirBlock(sph_system, "AirBody");
	AirMaterial* air_material = new AirMaterial();
	FluidParticles 	air_particles(air_block, air_material);
	/**
	 * @brief 	Particle and body creation of wall boundary.
	 */
	WallBoundary* wall_boundary = new WallBoundary(sph_system, "Wall");
	SolidParticles 		wall_particles(wall_boundary);
	/** topology */
	ComplexBodyRelation* water_air_complex = new ComplexBodyRelation(water_block, { air_block });
	ContactBodyRelation* water_wall_contact = new ContactBodyRelation(water_block, { wall_boundary });
	ComplexBodyRelation* air_water_complex = new ComplexBodyRelation(air_block, { water_block });
	ContactBodyRelation* air_wall_contact = new ContactBodyRelation(air_block, { wall_boundary });
	/**
	 * @brief 	Define all numerical methods which are used in this case.
	 */
	 /** Define external force. */
	Gravity		gravity(Vecd(0.0, -gravity_g));
	/**
	 * @brief 	Methods used for time stepping.
	 */
	 /** Initialize particle acceleration. */
	InitializeATimeStep		initialize_a_water_step(water_block, &gravity);
	InitializeATimeStep		initialize_a_air_step(air_block, &gravity);
	/**
	 * @brief 	Algorithms of fluid dynamics.
	 */
	 /** Evaluation of density by summation approach. */
	fluid_dynamics::DensitySummationFreeSurfaceComplex 	
		update_water_density_by_summation(water_air_complex->inner_relation_, water_wall_contact);
	fluid_dynamics::DensitySummationComplex
		update_air_density_by_summation(air_water_complex, air_wall_contact);
	fluid_dynamics::TransportVelocityCorrectionComplex
		air_transport_correction(air_water_complex, air_wall_contact);
	/** Time step size without considering sound wave speed. */
	fluid_dynamics::AdvectionTimeStepSize 	get_water_advection_time_step_size(water_block, U_max);
	fluid_dynamics::AdvectionTimeStepSize 	get_air_advection_time_step_size(air_block, U_max);
	/** Time step size with considering sound wave speed. */
	fluid_dynamics::AcousticTimeStepSize get_water_time_step_size(water_block);
	fluid_dynamics::AcousticTimeStepSize get_air_time_step_size(air_block);
	/** Pressure relaxation for water by using position verlet time stepping. */
	fluid_dynamics::PressureRelaxationRiemannWithWall 
		water_pressure_relaxation(water_air_complex->inner_relation_, water_wall_contact);
	fluid_dynamics::DensityRelaxationRiemannWithWall 
		water_density_relaxation(water_air_complex->inner_relation_, water_wall_contact);
	/** Extend Pressure relaxation is used for air. */
	fluid_dynamics::ExtendMultiPhasePressureRelaxationRiemannWithWall
		air_pressure_relaxation(air_water_complex, air_wall_contact, 2.0);
	fluid_dynamics::MultiPhaseDensityRelaxationRiemannWithWall
		air_density_relaxation(air_water_complex, air_wall_contact);
	/** Viscous acceleration. */
	fluid_dynamics::ViscousAccelerationMultiPhase
		air_viscou_acceleration(air_water_complex);
	fluid_dynamics::ViscousAccelerationMultiPhase
		water_viscou_acceleration(water_air_complex);
	/** Suface tension and wetting effects. */
	fluid_dynamics::FreeSurfaceIndicationComplex
		surface_detection(water_air_complex->inner_relation_, water_wall_contact);
	fluid_dynamics::ColorFunctionGradientComplex
		color_gradient(water_air_complex->inner_relation_, water_wall_contact);
	fluid_dynamics::ColorFunctionGradientInterplationInner
		color_gradient_interpolation(water_air_complex->inner_relation_);
	fluid_dynamics::SurfaceTensionAccelerationInner
		surface_tension_acceleration(water_air_complex->inner_relation_, tension_force);
	/** Wetting effects. */
	fluid_dynamics::SurfaceNormWithWall
		wetting_norm(water_wall_contact, contact_angle);
	/**
	 * @brief Output.
	 */
	In_Output in_output(sph_system);
	/** Output the body states. */
	WriteBodyStatesToVtu 		write_body_states(in_output, sph_system.real_bodies_);
	/** Output the body states for restart simulation. */
	RestartIO		restart_io(in_output, sph_system.real_bodies_);
	/** Pre-simulation*/
	sph_system.initializeSystemCellLinkedLists();
	sph_system.initializeSystemConfigurations();
	wall_particles.initializeNormalDirectionFromGeometry();
	/**
	 * @brief The time stepping starts here.
	 */
	 /** If the starting time is not zero, please setup the restart time step ro read in restart states. */
	if (sph_system.restart_step_ != 0)
	{
		GlobalStaticVariables::physical_time_ = restart_io.readRestartFiles(sph_system.restart_step_);
		water_block->updateCellLinkedList();
		air_block->updateCellLinkedList();
		water_air_complex->updateConfiguration();
		water_wall_contact->updateConfiguration();
		air_water_complex->updateConfiguration();
		air_wall_contact->updateConfiguration();
	}
	/** Output the start states of bodies. */
	write_body_states.WriteToFile(GlobalStaticVariables::physical_time_);
	/**
	 * @brief 	Basic parameters.
	 */
	size_t number_of_iterations = sph_system.restart_step_;
	int screen_output_interval = 100;
	int observation_sample_interval = screen_output_interval * 2;
	int restart_output_interval = screen_output_interval * 10;
	Real End_Time = 5.0; 	/**< End time. */
	Real D_Time = End_Time / 50;		/**< Time stamps for output of body states. */
	Real Dt = 0.0;			/**< Default advection time step sizes. */
	Real dt = 0.0; 			/**< Default acoustic time step sizes. */
	Real dt_a = 0.0;
	Real dt_f = 0.0;
	/** statistics for computing CPU time. */
	tick_count t1 = tick_count::now();
	tick_count::interval_t interval;
	tick_count::interval_t interval_computing_time_step;
	tick_count::interval_t interval_computing_pressure_relaxation;
	tick_count::interval_t interval_updating_configuration;
	tick_count time_instance;

	/**
	 * @brief 	Main loop starts here.
	 */
	while (GlobalStaticVariables::physical_time_ < End_Time)
	{
		Real integration_time = 0.0;
		/** Integrate time (loop) until the next output time. */
		while (integration_time < D_Time)
		{
			/** Acceleration due to viscous force and gravity. */
			time_instance = tick_count::now();
			initialize_a_water_step.parallel_exec();
			initialize_a_air_step.parallel_exec();

			Real Dt_f = get_water_advection_time_step_size.parallel_exec();
			Real Dt_a = get_air_advection_time_step_size.parallel_exec();
			Dt = SMIN(Dt_f, Dt_a);

			update_water_density_by_summation.parallel_exec();
			update_air_density_by_summation.parallel_exec();
			air_transport_correction.parallel_exec(Dt);

			air_viscou_acceleration.parallel_exec();
			water_viscou_acceleration.parallel_exec();

			surface_detection.parallel_exec();
			color_gradient.parallel_exec();
			color_gradient_interpolation.parallel_exec();
			wetting_norm.parallel_exec();
			surface_tension_acceleration.parallel_exec();

			interval_computing_time_step += tick_count::now() - time_instance;

			/** Dynamics including pressure relaxation. */
			time_instance = tick_count::now();
			Real relaxation_time = 0.0;
			while (relaxation_time < Dt)
			{
				Real dt_f = get_water_time_step_size.parallel_exec();
				Real dt_a = get_air_time_step_size.parallel_exec();
				dt = SMIN(SMIN(dt_f, dt_a), Dt);

				water_pressure_relaxation.parallel_exec(dt);
				air_pressure_relaxation.parallel_exec(dt);

				water_density_relaxation.parallel_exec(dt);
				air_density_relaxation.parallel_exec(dt);

				relaxation_time += dt;
				integration_time += dt;
				GlobalStaticVariables::physical_time_ += dt;
			}
			interval_computing_pressure_relaxation += tick_count::now() - time_instance;

			if (number_of_iterations % screen_output_interval == 0)
			{
				std::cout << std::fixed << std::setprecision(9) << "N=" << number_of_iterations << "	Time = "
					<< GlobalStaticVariables::physical_time_
					<< "	Dt = " << Dt << "	dt = " << dt << "\n";

				if (number_of_iterations % restart_output_interval == 0)
					restart_io.WriteToFile(Real(number_of_iterations));
			}
			number_of_iterations++;

			/** Update cell linked list and configuration. */
			time_instance = tick_count::now();
			
			water_block->updateCellLinkedList();
			water_air_complex->updateConfiguration();
			water_wall_contact->updateConfiguration();

			air_block->updateCellLinkedList();
			air_water_complex->updateConfiguration();
			air_wall_contact->updateConfiguration();

			interval_updating_configuration += tick_count::now() - time_instance;
		}


		tick_count t2 = tick_count::now();
		write_body_states.WriteToFile(GlobalStaticVariables::physical_time_);
		tick_count t3 = tick_count::now();
		interval += t3 - t2;

	}

	tick_count t4 = tick_count::now();

	tick_count::interval_t tt;
	tt = t4 - t1 - interval;
	std::cout << "Total wall time for computation: " << tt.seconds()
		<< " seconds." << std::endl;
	std::cout << std::fixed << std::setprecision(9) << "interval_computing_time_step ="
		<< interval_computing_time_step.seconds() << "\n";
	std::cout << std::fixed << std::setprecision(9) << "interval_computing_pressure_relaxation = "
		<< interval_computing_pressure_relaxation.seconds() << "\n";
	std::cout << std::fixed << std::setprecision(9) << "interval_updating_configuration = "
		<< interval_updating_configuration.seconds() << "\n";

	return 0;
}
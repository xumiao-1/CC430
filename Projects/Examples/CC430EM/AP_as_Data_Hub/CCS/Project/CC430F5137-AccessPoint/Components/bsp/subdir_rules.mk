################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Components/bsp/bsp.obj: /home/test/workspace/SimpliciTI-CCS-111/Components/bsp/bsp.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/home/test/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/bin/cl430" --cmd_file="/home/test/workspace/SimpliciTI-CCS-111/Projects/Examples/CC430EM/AP_as_Data_Hub/CCS/Project/../Configuration/smpl_nwk_config.dat" --cmd_file="/home/test/workspace/SimpliciTI-CCS-111/Projects/Examples/CC430EM/AP_as_Data_Hub/CCS/Project/../Configuration/Access_Point/smpl_config.dat"  -vmspx --use_hw_mpy=F5 --include_path="/home/test/ti/ccsv6/ccs_base/msp430/include" --include_path="/home/test/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.5/include" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/bsp" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/bsp/boards/CC430EM" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/bsp/drivers" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/mrfi" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/simpliciti/nwk" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/simpliciti/nwk_applications" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/common/platform_ind" --include_path="/home/test/workspace/SimpliciTI-CCS-111/Components/common/platform" -g --define=__CC430F5137__ --define=MRFI_CC430 --diag_warning=225 --silicon_errata=CPU18 --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=minimal --preproc_with_compile --preproc_dependency="Components/bsp/bsp.pp" --obj_directory="Components/bsp" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '



################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv4 --code_state=32 -me --include_path="/Users/mattstevenson/Desktop/ECE266-Labs/project/src" --include_path="/Applications/ti/TivaWare_C_Series-2.1.4.178" --include_path="/Users/mattstevenson/Desktop/ECE266-Labs/Util" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/Applications/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/bin/armcl" -mv4 --code_state=32 -me --include_path="/Users/mattstevenson/Desktop/ECE266-Labs/project/src" --include_path="/Applications/ti/TivaWare_C_Series-2.1.4.178" --include_path="/Users/mattstevenson/Desktop/ECE266-Labs/Util" --include_path="/Applications/ti/ccsv8/tools/compiler/ti-cgt-arm_18.1.4.LTS/include" -g --diag_warning=225 --diag_wrap=off --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '



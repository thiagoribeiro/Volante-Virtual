################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../FSMclass.cpp \
../FSMstate.cpp \
../GranTurismOgre.cpp \
../Veiculo.cpp \
../controle.cpp \
../line3D.cpp \
../metodos.cpp 

OBJS += \
./FSMclass.o \
./FSMstate.o \
./GranTurismOgre.o \
./Veiculo.o \
./controle.o \
./line3D.o \
./metodos.o 

CPP_DEPS += \
./FSMclass.d \
./FSMstate.d \
./GranTurismOgre.d \
./Veiculo.d \
./controle.d \
./line3D.d \
./metodos.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/aluno/Downloads/ogre_src_v1-7-3//OgreMain/include" -I"/home/aluno/Downloads/ogre_src_v1-7-3//include" -I"/home/aluno/Downloads/ogre_src_v1-7-3//Samples/Common/include" -I/usr/include/OIS -I/home/aluno/Downloads/ogre_src_v1-7-3/ogreaddons/ogreode/Core/include/ -I/home/aluno/Downloads/ogre_src_v1-7-3/ogreaddons/ogreode/prefab/include/ -I/home/aluno/Downloads/ogre_src_v1-7-3/ogreaddons/ogreode/loader/include/ -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '



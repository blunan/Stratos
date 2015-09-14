#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/application-helper.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/neighborhood-application.o \
	${OBJECTDIR}/ontology-application.o \
	${OBJECTDIR}/position-application.o \
	${OBJECTDIR}/results-application.o \
	${OBJECTDIR}/route-application.o \
	${OBJECTDIR}/search-application.o \
	${OBJECTDIR}/search-error-header.o \
	${OBJECTDIR}/search-request-header.o \
	${OBJECTDIR}/search-response-header.o \
	${OBJECTDIR}/service-application.o \
	${OBJECTDIR}/service-error-header.o \
	${OBJECTDIR}/service-request-response-header.o \
	${OBJECTDIR}/stratos.o \
	${OBJECTDIR}/type-header.o \
	${OBJECTDIR}/utilities.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stratos

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stratos: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stratos ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/application-helper.o: application-helper.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/application-helper.o application-helper.cc

${OBJECTDIR}/main.o: main.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cc

${OBJECTDIR}/neighborhood-application.o: neighborhood-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/neighborhood-application.o neighborhood-application.cc

${OBJECTDIR}/ontology-application.o: ontology-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/ontology-application.o ontology-application.cc

${OBJECTDIR}/position-application.o: position-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/position-application.o position-application.cc

${OBJECTDIR}/results-application.o: results-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/results-application.o results-application.cc

${OBJECTDIR}/route-application.o: route-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/route-application.o route-application.cc

${OBJECTDIR}/search-application.o: search-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/search-application.o search-application.cc

${OBJECTDIR}/search-error-header.o: search-error-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/search-error-header.o search-error-header.cc

${OBJECTDIR}/search-request-header.o: search-request-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/search-request-header.o search-request-header.cc

${OBJECTDIR}/search-response-header.o: search-response-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/search-response-header.o search-response-header.cc

${OBJECTDIR}/service-application.o: service-application.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service-application.o service-application.cc

${OBJECTDIR}/service-error-header.o: service-error-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service-error-header.o service-error-header.cc

${OBJECTDIR}/service-request-response-header.o: service-request-response-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/service-request-response-header.o service-request-response-header.cc

${OBJECTDIR}/stratos.o: stratos.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/stratos.o stratos.cc

${OBJECTDIR}/type-header.o: type-header.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/type-header.o type-header.cc

${OBJECTDIR}/utilities.o: utilities.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/utilities.o utilities.cc

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/stratos

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc

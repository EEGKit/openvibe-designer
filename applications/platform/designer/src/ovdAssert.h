#pragma once

#include <exception>
#include <stdexcept>
#include <fs/Files.h>

#include <openvibe/ovAssert.h>
#include <openvibe/kernel/error/ovIErrorManager.h>
#include <openvibe/kernel/error/ovIError.h>

#define OV_ERROR_D(description, type, returnValue) OV_ERROR(description, type, returnValue, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_DRF(description, type) OV_ERROR(description, type, false, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_DRZ(description, type) OV_ERROR(description, type, 0, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_DRU(description, type) OV_ERROR(description, type, OV_UndefinedIdentifier, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_DRV(description, type) OV_ERROR(description, type, void(), m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_DRN(description, type) OV_ERROR(description, type, nullptr, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())

#define OV_ERROR_UNLESS_D(expression, description, type, returnValue) OV_ERROR_UNLESS(expression, description, type, returnValue, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_UNLESS_DRF(expression, description, type) OV_ERROR_UNLESS(expression, description, type, false, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_UNLESS_DRZ(expression, description, type) OV_ERROR_UNLESS(expression, description, type, 0, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_UNLESS_DRU(expression, description, type) OV_ERROR_UNLESS(expression, description, type, OV_UndefinedIdentifier, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_UNLESS_DRV(expression, description, type) OV_ERROR_UNLESS(expression, description, type, void() , m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())
#define OV_ERROR_UNLESS_DRN(expression, description, type) OV_ERROR_UNLESS(expression, description, type, nullptr, m_KernelContext.getErrorManager(), m_KernelContext.getLogManager())

#define OV_FATAL_D(description, type) OV_FATAL(description, type, m_KernelContext.getLogManager())
#define OV_FATAL_UNLESS_D(expression, description, type) OV_FATAL_UNLESS(expression, description, type, m_KernelContext.getLogManager())

#ifndef _MSC_VER
#define NOEXCEPT noexcept
#else
#define NOEXCEPT
#endif

class DesignerException final : public std::runtime_error
{
public:
	DesignerException(OpenViBE::Kernel::IErrorManager& errorManager)
		: std::runtime_error("Designer caused an exception")
		  , m_ErrorManager(errorManager) {}

	const char* what() const NOEXCEPT override
	{
		return m_ErrorManager.getLastErrorString();
	}

	std::string getErrorString() const
	{
		std::string errorMessage;
		const OpenViBE::Kernel::IError* error = m_ErrorManager.getLastError();
		while (error)
		{
			char location[1024];
			FS::Files::getFilename(error->getErrorLocation(), location);
			errorMessage += "Message: " + std::string(error->getErrorString()) +
				"\nFile: " + location + "\n";
			error = error->getNestedError();
		}
		m_ErrorManager.releaseErrors();
		return errorMessage;
	}

	void releaseErrors() NOEXCEPT
	{
		m_ErrorManager.releaseErrors();
	}

	OpenViBE::Kernel::IErrorManager& m_ErrorManager;
};

#define OV_EXCEPTION_D(description, type) \
do { \
	m_KernelContext.getErrorManager().pushErrorAtLocation( \
		type, \
		static_cast<const OpenViBE::ErrorStream&>(OpenViBE::ErrorStream() << description).str().c_str(), \
		__FILE__, __LINE__ \
	); \
	m_KernelContext.getLogManager() << OpenViBE::Kernel::LogLevel_Fatal \
			   << "[Error description] = " \
			   << description \
			   << "; [Error code] = " \
			   << static_cast<unsigned int>((type)) \
			   << "\n"; \
	throw DesignerException(m_KernelContext.getErrorManager()); \
} \
while (0)

#define OV_EXCEPTION_UNLESS_D(expression, description, type) \
do { \
   if (!(expression)) \
   { \
	   OV_EXCEPTION_D(description, type); \
   } \
} \
while (0)

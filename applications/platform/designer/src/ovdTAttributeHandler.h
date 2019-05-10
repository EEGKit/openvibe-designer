#pragma once

#include "ovd_base.h"

namespace OpenViBEDesigner
{
	class TAttributeHandler
	{
	public:

		TAttributeHandler(OpenViBE::Kernel::IAttributable& rAttributable);
		TAttributeHandler(const OpenViBE::Kernel::IAttributable& rAttributable);

		template <class T>
		bool addAttribute(const OpenViBE::CIdentifier& rAttributeIdentifier, const T& rValue) const;

		bool removeAttribute(const OpenViBE::CIdentifier& rAttributeIdentifier);

		bool removeAllAttributes();

		template <class T>
		T getAttributeValue(const OpenViBE::CIdentifier& rAttributeIdentifier) const;

		template <class T>
		bool setAttributeValue(const OpenViBE::CIdentifier& rAttributeIdentifier, const T& rValue);

		bool hasAttribute(const OpenViBE::CIdentifier& rAttributeIdentifier) const;

		bool hasAttributes() const;

	protected:

		const OpenViBE::Kernel::IAttributable* m_pConstAttributable;
		OpenViBE::Kernel::IAttributable* m_pAttributable;
	};
};

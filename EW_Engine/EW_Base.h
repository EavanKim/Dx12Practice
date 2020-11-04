#pragma once

namespace EW
{
	class EW_Base
	{
	public:
		inline const long long GetRef();
		inline const long long InstanceRelease();

	protected:


	private:
		inline const long long IncRef();
		inline const long long DecRef();

		long long m_lReferrencCount = 0;
	};
	#include "EW_Base.inl"
}

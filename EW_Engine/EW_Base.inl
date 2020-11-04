#ifndef EW_BASE_INL__
#define EW_BASE_INL__

const long long EW::EW_Base::GetRef()
{
	return m_lReferrencCount;
}

const long long EW::EW_Base::InstanceRelease()
{
	if (0 == DecRef())
	{
		delete this;

		return 0;
	}
	else
		return m_lReferrencCount;
}

const long long EW::EW_Base::IncRef()
{
	return ++m_lReferrencCount;
}

const long long EW::EW_Base::DecRef()
{
	return --m_lReferrencCount;
}

#endif
#pragma once

namespace ES
{
	// Event base struct that all events should inherit from
	struct Event
	{
		virtual ~Event() = default;
	};
}

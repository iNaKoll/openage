// Copyright 2014-2014 the openage authors. See copying.md for legal info.

#ifndef OPENAGE_LOG_CONFIGURE_H_
#define OPENAGE_LOG_CONFIGURE_H_

#include "log.h"

namespace openage { 
	// Forward declaration of type/class used as logger tags.
	// These forward declarations are only necessary to configure the logger tags properly
	// (i.e change their default behaviour) and not to actually use them (to log some messages).
	struct GameMain;
	struct Texture;
	
	namespace audio {
		struct OpusInMemoryLoader;
		struct DynamicResource;
		struct AudioManager;
		struct OpusDynamicLoader;
	}

	struct Font;

	namespace util {
		struct Error;
		struct ExternalProfiler;
	}
	
namespace log {
	// Log configuration 
	
	// Policy based logger, use noop_printer to disable the associated information
	// Once a logger has been defined, don't forget to register it through the specialization 
	// of configure<LogManager>::registered_loggers (see below).

	using ConsoleLogger = AsyncLogger<
			  cout
			, time_printer
			, tag_printer
			, noop_printer
			, colored_loglevel_printer
			, origin_printer
	>;

	/* Some examples of log configuration */
	/*
	OPENAGE_TPL_STRING(openage_log, "openage.log");
	using FileLogger = AsyncLogger<
			  fout<openage_log>
			, datetime_printer
			, tag_printer
			, thread_printer
			, loglevel_printer   // txt editors wont support colors
			, origin_printer
			, type_list<>
			, type_list<YourMom> // we don't want YourMom there
	>;

	// YourMom deserves its own log file
	// Whenever we write something like :
	// 	log::tmsg(YourMom, "Don't forget to close the door.")
	// this will push a new log entry into this logger.
	// All others log tags will be ignored by this logger.
	OPENAGE_TPL_STRING(yourmom_log, "yourmom.log");
	using YourMomFileLogger = AsyncLogger<
			  fout<yourmom_log>
			, datetime_printer
			, noop_printer      // we don't need to print the tags as YourMom is the only one there
			, thread_printer
			, loglevel_printer
			, origin_printer
			, type_list<YourMom>  // only accepts YourMom logger tag!
	>;
	*/

	// This specialization of 'configure' for the LogManager has to define the registered loggers list used throughout the whole application.
	// This logger list has to be maintained each time a new logger is defined.
	template <>
	struct configure<LogManager> {
		using registered_loggers = type_list<ConsoleLogger>;
	};

	// By default _any_ logger tag verbosity level is set to LogLevel::DBG.
	// However if NDEBUG is defined the default verbosity level is LogLevel::MSG.
	// If the default behaviour is good for you, there is no need to configure explicilty 
	// a logger tag verbosity level and you don't need to edit this file to say anything 
	// about your logger tag. Nevertheless, there's nothing wrong being explicit... :)
	
	// Below, this configure specialization sets the default logger tag (openage::log::root) verbosity level. 
	// This tag is used by default (with log::dbg, log::msg, etc...), when the user doesn't provides 
	// a logger tag (using log::tdbg(LoggerTag, ...) inseated).
	template <>
	struct configure<root> {
#		ifndef NDEBUG
		static const constexpr LogLevel log_level = LogLevel::DBG;
#		else
		static const constexpr LogLevel log_level = LogLevel::MSG;
#		endif
	};

	/* Example of YourMom log configuration */
	/*
	// Defines YourMom logger tag verbosity level
	template <>
	struct configure<YourMom> {
		// You should always listen to YourMom but she's just talking too much
		static const constexpr LogLevel log_level = LogLevel::IMP;
	};
	*/

	// Defines some logger tags verbosity level
	template <>
	struct configure<audio::OpusInMemoryLoader> {
		static const constexpr LogLevel log_level = LogLevel::MSG;
	};

	template <>
	struct configure<GameMain> {
		static const constexpr LogLevel log_level = LogLevel::IMP;
	};

	template <>
	struct configure<Texture> {
		static const constexpr LogLevel log_level = LogLevel::MSG;
	};
} // namespace log
} // namespace openage

#endif


// Inclusion in build system conditional on USE_SPEECHDISPATCHER
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Speech.h"

CSpeech::CSpeech() : m_speaker(NULL) {
}

CSpeech::~CSpeech() {
	if (m_speaker) spd_close(m_speaker);
}


bool CSpeech::Init() {
	if (m_speaker) return true;

	m_speaker = spd_open("dasher", NULL, NULL, SPD_MODE_SINGLE);

	return m_speaker != NULL;
}

void CSpeech::Speak(const std::string &strText, bool bInterrupt, const std::string &lang) {
	if (!Init()) {
		return;
	}
	if (!bInterrupt) {
		if (!lang.empty()) {
			// Speechd only accepts 2-letter language codes and
			// silently ignores anything else.
			std::string shortLang = lang.substr(0, 2);
			spd_set_language(m_speaker, shortLang.c_str());
		}
		if (!strText.empty()) {
			spd_say(m_speaker, SPD_TEXT, strText.c_str());
		}
	} else {
		spd_stop(m_speaker);
	}	
}

#!/bin/sh
#
#   utpdf/utps
#   margin-aware converter from utf-8 text to PDF/PostScript
# 
#   Copyright (c) 2021 by Akihiro SHIMIZU
# 
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
# 
#   http://www.apache.org/licenses/LICENSE-2.0
# 
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
# 

if [ $# -eq 0 ]; then
    echo "USAGE: font_candidate.sh <lang>"
    echo "    <lang> : ISO639-1 Alpha-2 language code (ja, en, fr, de,...)"
    exit 1
fi

fc-list :lang=$1 | awk -F: '{ print $2; }'| sed -e s/^\ *// | tr "," "\n" | \
 egrep '^[A-Za-z0-9\ \\\-]+$' | sed -e '{
	s/\ Thin$//
	s/\ Ultralight$//
	s/\ Light$//
	s/\ Semilight$//
	s/\ Book$//
	s/\ Normal$//
	s/\ Medium$//
	s/\ Semibold$//
	s/\ Bold$//
	s/\ Ultrabold$//
	s/\ Heavy$//
	s/\ Ultraheavy$//
	s/\ Regular$//
	s/\ Demibold$//
	s/\ Extrabold$//
	s/\ GB$//
	s/\ W[0-9]$//
	s/\ SC$//
	s/\ TC$//
}' | sort | uniq 

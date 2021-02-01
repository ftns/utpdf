#!/bin/sh
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

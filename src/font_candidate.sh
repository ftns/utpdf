#!/bin/sh
fc-list :lang=ja | awk -F: '{ print $2; }'| sed -e s/^\ *// | tr "," "\n" | \
 egrep '^[A-Za-z0-9\ ]+$' | sed -e '{
	s/\ Thin$//i
	s/\ Ultralight$//i
	s/\ Light$//i
	s/\ Semilight$//i
	s/\ Book$//i
	s/\ Normal$//i
	s/\ Medium$//i
	s/\ Semibold$//i
	s/\ Bold$//i
	s/\ Ultrabold$//i
	s/\ Heavy$//i
	s/\ Ultraheavy$//i
	s/\ Regular$//i
	s/\ Demibold$//i
	s/\ Extrabold$//i
	s/\ GB$//i
	s/\ W[0-9]$//i
}' | sort | uniq 

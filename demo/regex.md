
# Regex

## syntax

`path_regex <prefix> <regex> <handler> {<config>}`

## example


	path /demo StaticHandler {
		root demo;
	}
	path_regex /demo \.md MarkdownHandler {
		root demo;
	}

## details

- regex matches take priority over fixed matches
- ecmascript syntax
- {, }, " are not supported yet



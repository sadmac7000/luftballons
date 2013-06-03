# Hacking on Luftballons #

Here are the contribution guidelines for Luftballons. This should stay brief,
but it's definitely important.


## Communication ##

Luftballons has its own IRC channel, `#luftballons`, on the [freenode
network](http://freenode.net). For now we'll let this handle all
Luftballons-related content, including
development discussion.

The mailing list is `devel@lists.luftengine.org`. This should be handling
development traffic exclusively.


## Commit Guidelines ##

Git has been around long enough that people more or less know by now what makes
sense, but we'll go over the important and/or quirky stuff. Commit messages
should be:

* One line summarizing the commit briefly
* A blank line
* Optionally several paragraphs describing your change in detail
* A blank line (unless that's two in a row)
* A standard `Signed-off-by` section.

The first line should completely, yet concisely capture your commit's entire
effect. The description paragraphs should explain rationale and consequences.
If the first line can't capture your change, your commit is too big and you
should split it up.

Commit messages, like everything else, should wrap at 80 characters. Some git
projects prefer 60 for some output, but we use big enough terminals that the
wrapping isn't so much of an issue as easy visual scanning (just about anyone
working on a game engine must, by definition, be using a graphical
environment). If your editor is already set up to wrap git commits at 60, then
it's probably no big deal, but if it isn't go ahead and use all 80.

The `Signed-off-by` section is a Linux kernel convention. Fortunately the
kernel's close relationship with git means there's some inbuilt support for it.
Invoking git with the `-s` option will place your signoff line in the commit
message in the editor. Usually this will be all you need. If the commit has
multiple authors, or if more than one person is asserting maintainer-like
responsibility for this change, you should add another signoff line. You should
only put a signoff line on code whose origin you can vouch for. Luftballons
uses signoff lines to indicate responsibility for the code, but they were
originally designed with a (somewhat weak) legal significance, and we'll try to
preserve that.

Other kernel-isms like `Acked-by` and `Reviewed-by` won't be defined here, but
you can use them if you like.


### Commit Contents ###

A commit should contain "minimal, atomic changes." Atomic is easy to define:

* Does the code still build with all affected options turned on?
* Do all advertised APIs work correctly?
* If introducing a new internal API for features to be implemented later, is it
  expected to work correctly?
* Are we otherwise free of added dead code?

Minimal is a bit more subjective, but what we said above is probably about as
good a guideline as any: If you can't capture the whole commit's scope in the
first line it's probably too big. There may be some shuffling for long words,
but you should be able to use judgment.


## Submitting commits ##

Changes should generally pass through the mailing list. For large changesets
you can use `git request-pull` and for smaller ones you can use `git
send-email`. Request-pull should have the URL to pass directly to the `git
pull` command. In both cases the results should go to the mailing list. Unless
clarified here, the defaults for these commands should suffice.

GitHub pull requests are also allowed, but you should notify the mailing list
when you submit them. Be aware that depending on how things go, we may disallow
them later. You're safer just using the list.


## Languages ##

Luftballons is written in "Gnu99" standard C, which makes it a bit
GCC-dependent. If you have cause to build it on another compiler, that can be
your first place to contribute! If there's serious continued interest we'll
happily amend these guidelines to make that easier.

Due to compatibility, some of the COLLADA code is written in C++. **Do not
submit additional C++ to Luftballons**. This is a C project. C++ is only there
when we need it to talk to a library (and it isn't very good C++). Even in that
case, we'd rather pick a different library when an alternative exists.

Should any tasks emerge that are better suited to a scripting language, Python
is preferred.


## C Style ##

Where not contradicted here, Linux kernel official style is generally
applicable.

* Indentation is eight-space hard tabs.
* Code wraps at 80 characters.
* Function argument lists appear with the parenthesis immediately after the
  function name.
* `if` and `switch` statements and loop constructs have a space between the
  name and the beginning of the list arguments.
* The open curly brace for a block is on the same line as the conditional or
  loop statement.
* The open curly brace for a function is on its own line.
* If a loop or conditional can omit curly braces, it does.
* If a loop or conditional contains a loop or conditional that needs curly
  braces, the outer loop or conditional also gets curly braces.
* In a chain of `if`-`else` statements, if any of the conditional bodies need
  curly braces, they all get curly braces.
* `struct`s and `enum`s are always `typedef`d. `typedef`s end in `_t`
* When argument lists are broken into multiple lines, the next line starts
  aligned just after the opening parenthesis.
* Function predeclarations appear only when necessary. Functions should be
  defined in an order that prevents that as much as possible.
* Function definitions have the type on a separate line from the function name.

Lastly, code should *always* be written to have the *minimum amount of
nesting*. So, for example:

	if (foo)
		a++;
	else
		return 1;

Should be written:

	if (! foo)
		return 1;

	a++;

And:

	int
	somefunc(void)
	{
		if (some_condition) {
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
			/* .... Lots of code .... */
		}

		return 1;
	}

Should be written:

	int
	somefunc(void)
	{
		if (! some_condition)
			return 1;

		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */
		/* .... Lots of code .... */

		return 1;
	}


## Comments ##

We generally avoid the `//` convention for comments. Stick to `/* */`. Structs
and functions should have a documentation comment. The format looks like this:

	/**
	 * A description of the struct or function.
	 *
	 * argument1: Description of the argument or member.
	 * argument2: Same.
	 *
	 * Returns: A value of some kind. Omit for non-functions.
	 **/

We are deliberately loose with the second two sections: they are frequently
omitted. Generally, they shouldn't be there if their contents are obvious. For
example, don't do this:

	/**
	 * Frob a dingleberry.
	 *
	 * dingleberry: The dingleberry to frob.
	 **/

And similarly, don't do this:

	/**
	 * Get the number of eggs in a basket.
	 *
	 * Returns: The number of eggs in a basket.
	 **/

If the description clearly covers it, don't list the arguments or members or
return value.

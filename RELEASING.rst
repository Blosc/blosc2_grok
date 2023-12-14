Releasing a version
===================

Preliminaries
-------------

- Make sure that ``RELEASE_NOTES.md`` and ``ANNOUNCE.md`` are up to
  date with the latest news in the release.

- Check that *VERSION* symbols in blosc2_grok/__init__.py
  *and* pyproject.toml contain the correct info.

- Commit the changes with::

    $ git commit -a -m "Getting ready for release X.Y.Z"
    $ git push


Testing
-------

Follow the steps in README-DEVELOPERS.md file for locally creating and
installing the wheel, then test it::

  $ python -m pytest

Also::

  $ cd examples
  $ python roundtrip.py MI04_020751.tif
  $ cd ..


Tagging
-------

- Create a tag ``X.Y.Z`` from ``main``.  Use the next message::

    $ git tag -a vX.Y.Z -m "Tagging version X.Y.Z"

- Push the tag to the github repo::

    $ git push --tags

- If you happen to have to delete the tag, such as artifacts demonstrates a fault, first delete it locally::

    $ git tag --delete vX.Y.Z

  and then remotely on Github::

    $ git push --delete origin vX.Y.Z

- After the CI uploads wheels in PyPI, do a manual wheel for the Apple ARM64 architecture::

    $ python -m cibuildwheel --only 'cp312-macosx_arm64'

  and upload it to PyPI::

    $ python -m twine upload wheelhouse/blosc2_grok-X.Y.Z-py3-none-macosx_11_0_arm64.whl

  (note that you will need to have a PyPI account and be a maintainer of the blosc2_grok project there).

- Create a new release visiting https://github.com/Blosc/blosc2_grok/releases/new
  and add the release notes copying them from `RELEASE_NOTES.md` document.


Check documentation
-------------------

Check that the `README.md` and `README-DEVELOPERS.md` are consistent with this new release.


Announcing
----------

- Send an announcement to the blosc and comp.compression mailing lists.
  Use the ``ANNOUNCE.md`` file as skeleton (likely as the definitive version).

- Tweet/toot about it from the @Blosc2 account.


Post-release actions
--------------------

- Edit *VERSION* symbols in blosc2_grok/__init__.py *and* pyproject.toml in main to increment the
  version to the next minor one (i.e. X.Y.Z --> X.Y.(Z+1).dev).

- Create new headers for adding new features in ``RELEASE_NOTES.md``
  and add this place-holder instead:

  #XXX version-specific blurb XXX#

- Commit the changes::

  $ git commit -a -m"Post X.Y.Z release actions done"
  $ git push

That's all folks!

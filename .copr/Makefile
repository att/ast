mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

srpm:
	dnf -y install git-core sed
	$(current_dir)/prep.sh
	rpmbuild -bs -D "dist %{nil}" -D "_sourcedir packaging/fedora" -D "_srcrpmdir $(outdir)" --nodeps packaging/fedora/ksh.spec

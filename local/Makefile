BUILDS=api esol
TARGETS=all clean

$(TARGETS):
	for b in $(BUILDS) ; do $(MAKE) -C $$b $@ ; done

$(BUILDS):
	for t in $(TARGETS) ; do $(MAKE) -C $@ $$t ; done

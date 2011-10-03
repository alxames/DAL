import BeamFormedFile

# create the file
f = BeamFormedFile.BeamFormedFile("foo.h5", BeamFormedFile.BeamFormedFile.CREATE)

# create an attribute
a = BeamFormedFile.AttributeString(f.group(), "MY_ATTRIBUTE")
a.set("hello world!")

# create and destroy a second attribute
b = BeamFormedFile.AttributeString(f.group(), "FAULTY_ATTRIBUTE")
b.set("hello world!")
b.remove()

# create a dataset
d = BeamFormedFile.HDF5DatasetBaseComplexFloat(f.group(), "MY_DATASET")
d.create([4,4],[4,4])

# insert some data points
d.setScalar([0,0],1+2j)
d.setScalar([1,0],2+3j)
d.setScalar([0,1],3+4j)

# flushing is not needed, but can be done explicitly
f.flush()


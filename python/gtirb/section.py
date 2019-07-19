import Section_pb2

from gtirb.node import Node


class Section(Node):
    """
    Represents a named section of the binary.

    Does not directly store the contents of the section, which are
    kept in ImageByteMap.
    """

    def __init__(self, name='', address=0, size=0, uuid=None):
        super().__init__(uuid)
        self.name = name
        self.address = address
        self.size = size

    @classmethod
    def _decode_protobuf(cls, section, uuid):
        return cls(section.name, section.address, section.size, uuid)

    def to_protobuf(self):
        """
        Returns protobuf representation of the object

        :returns: protobuf representation of the object
        :rtype: protobuf object

        """
        proto_section = Section_pb2.Section()
        proto_section.uuid = self.uuid.bytes
        proto_section.name = self.name
        proto_section.address = self.address
        proto_section.size = self.size
        return proto_section
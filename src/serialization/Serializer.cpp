/*
 * Serializer.cpp
 *
 *  Created on: Mar 29, 2015
 *      Author: bog
 */

#include <boglfw/serialization/Serializer.h>
#include <boglfw/serialization/BigFile.h>
#include <boglfw/serialization/BinaryStream.h>
#include <boglfw/utils/log.h>

#include <memory>
#include <sstream>

std::map<int, Serializer::DeserializeFuncType> Serializer::mapTypesToFuncs_;

void Serializer::setDeserializationObjectMapping(int objType, DeserializeFuncType func) {
	mapTypesToFuncs_[objType] = func;
}

Serializer::Serializer() {
}

Serializer::~Serializer() {
}

void Serializer::queueObject(serializable_wrap &&obj) {
	serializationQueue_.push_back(obj);
}

std::string Serializer::getObjectTypeString(int type) {
#warning "this method must be pure virtual and user override it with his types"
	/*switch (type) {
	case SerializationObjectTypes::BUG:
		return "bug";
	case SerializationObjectTypes::GAMETE:
		return "gamete";
	case SerializationObjectTypes::GENOME:
		return "genome";
	case SerializationObjectTypes::WALL:
		return "wall";
	case SerializationObjectTypes::FOOD_DISPENSER:
		return "food-dispenser";
	default:*/
		return "UNKNOWN_TYPE";
	//}
}

bool Serializer::serializeToFile(const std::string &path) {
	LOGPREFIX("Serializer");
	BinaryStream masterStream(serializationQueue_.size() * 50); // estimate about 50 bytes per entry in master
	std::vector<std::unique_ptr<BinaryStream>> vecStreams;
	std::vector<std::string> vecFilenames;
	int fileIndex = 1;
	for (auto &e : serializationQueue_) {
		int objType = e.getType();
		assert(mapTypesToFuncs_.find(objType) != mapTypesToFuncs_.end());
		masterStream << objType;
		std::stringstream pathBuild;
		pathBuild << getObjectTypeString(e.getType()) << fileIndex << ".data";
		vecFilenames.push_back(pathBuild.str());
		masterStream << pathBuild.str();
		std::unique_ptr<BinaryStream> fileStream(new BinaryStream(100));
		e.serialize(*fileStream);
		vecStreams.push_back(std::move(fileStream));

		fileIndex++;
	}
	serializationQueue_.clear();
	BigFile bigFile;
	bigFile.addFile("master", masterStream.getBuffer(), masterStream.getSize());
	for (unsigned i=0; i<vecStreams.size(); i++) {
		bigFile.addFile(vecFilenames[i], vecStreams[i]->getBuffer(), vecStreams[i]->getSize());
		vecStreams[i].reset();
	}
	return bigFile.saveToDisk(path);
}

bool Serializer::deserializeFromFile(const std::string &path) {
	LOGPREFIX("Serializer");
	LOGLN("Deserializing file \""<<path<<"\"...");
	BigFile bigFile;
	if (!bigFile.loadFromDisk(path)) {
		LOGLN("WARNING: BigFile loading FAILED at: " << path);
		return false;
	}
	BigFile::FileDescriptor master = bigFile.getFile("master");
	if (master.pStart == nullptr) {
		LOGLN("WARNING: BigFile MASTER record is empty at: " << path);
		return false;
	}
	try {
		BinaryStream masterStream(master.pStart, master.size);
		while (!masterStream.eof()) {
			int type;
			std::string filename;
			masterStream >> type >> filename;
			DeserializeFuncType deserializeFunc = mapTypesToFuncs_[type];
			if (!deserializeFunc) {
				LOGLN("WARNING: no known method to deserialize object type (" <<(int)type<<") at: " << path<<"/"<<filename);
				return false;
			}
			LOGLN("Deserializing entity type: " << getObjectTypeString(type));
			BigFile::FileDescriptor fileDesc = bigFile.getFile(filename);
			if (!fileDesc.pStart) {
				LOGLN("WARNING: file with internal name ("<<filename<<") has size zero! skipping...");
				continue;
			}
			BinaryStream fileStream(fileDesc.pStart, fileDesc.size);
			deserializeFunc(fileStream);
		}
		LOGLN("File deserialization SUCCESSFUL.");
		return true;
	} catch (std::runtime_error &e) {
		ERROR("EXCEPTION: " << e.what());
		return false;
	}
}


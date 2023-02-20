#pragma once
#include "Hooks.h"
#include "iBaseClientDLL.h"
#include "steam_sdk/isteamgamecoordinator.h"
#define CASTMSG(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)
#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo

#define MAKE_TAG(FIELD_NUMBER, TYPE) static_cast<uint32_t>(((FIELD_NUMBER) << Field::TagTypeBits) | (TYPE))

#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientReserve 9107
#define k_EMsgGCClientWelcome 4004
#define k_EMsgGCClientHello 4006
#define k_EMsgGCAdjustItemEquippedState 1059
#define k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello 9109
#define k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello 9110

// ProfileChanger
struct MatchmakingGC2ClientHello {
	static const uint32_t commendation = 8;
	static const uint32_t ranking = 7;
	static const uint32_t player_level = 17;
	static const uint32_t player_xp = 18;
	static const uint32_t penalty_seconds = 4;
	static const uint32_t penalty_reason = 5;
};
struct PlayerCommendationInfo {
	static const uint32_t cmd_friendly = 1;
	static const uint32_t cmd_teaching = 2;
	static const uint32_t cmd_leader = 4;
};
struct PlayerRankingInfo {
	static const uint32_t rank_id = 2;
	static const uint32_t wins = 3;
};

// InvChanger
struct SubscribedType {
	static const uint32_t type_id = 1;
	static const uint32_t object_data = 2;
};

struct CMsgSOCacheSubscribed {
	static const uint32_t objects = 2;
};

struct CMsgClientWelcome {
	static const uint32_t outofdate_subscribed_caches = 3;
};

struct CSOEconItem {
	static const uint32_t id = 1;
	static const uint32_t account_id = 2;
	static const uint32_t inventory = 3;
	static const uint32_t def_index = 4;
	static const uint32_t quantity = 5;
	static const uint32_t level = 6;
	static const uint32_t quality = 7;
	static const uint32_t flags = 8;
	static const uint32_t origin = 9;
	static const uint32_t custom_name = 10;
	static const uint32_t attribute = 12;
	static const uint32_t in_use = 14;
	static const uint32_t style = 15;
	static const uint32_t original_id = 16;
	static const uint32_t equipped_state = 18;
	static const uint32_t rarity = 19;
};

struct CMsgAdjustItemEquippedState {
	static const uint32_t item_id = 1;
	static const uint32_t new_class = 2;
	static const uint32_t new_slot = 3;
};

struct CSOEconItemEquipped {
	static const uint32_t new_class = 1;
	static const uint32_t new_slot = 2;
};

struct CSOEconItemAttribute {
	static const uint32_t def_index = 1;
	static const uint32_t value = 2;
	static const uint32_t value_bytes = 3;
};
enum FieldType {
	TYPE_DOUBLE = 1,
	TYPE_FLOAT = 2,
	TYPE_INT64 = 3,
	TYPE_UINT64 = 4,
	TYPE_INT32 = 5,
	TYPE_FIXED64 = 6,
	TYPE_FIXED32 = 7,
	TYPE_BOOL = 8,
	TYPE_STRING = 9,
	TYPE_GROUP = 10,
	TYPE_MESSAGE = 11,
	TYPE_BYTES = 12,
	TYPE_UINT32 = 13,
	TYPE_ENUM = 14,
	TYPE_SFIXED32 = 15,
	TYPE_SFIXED64 = 16,
	TYPE_SINT32 = 17,
	TYPE_SINT64 = 18,
	MAX_FIELD_TYPE = 18,
};

struct Field
{
	struct Tag
	{
		unsigned field;
		unsigned type;
	};

	enum WireType {
		WIRETYPE_VARINT = 0,
		WIRETYPE_FIXED64 = 1,
		WIRETYPE_LENGTH_DELIMITED = 2,
		WIRETYPE_START_GROUP = 3,
		WIRETYPE_END_GROUP = 4,
		WIRETYPE_FIXED32 = 5,
	};

	const WireType kWireTypeForFieldType[MAX_FIELD_TYPE + 1] = {
		static_cast<WireType>(-1),  // invalid
		WIRETYPE_FIXED64,           // TYPE_DOUBLE
		WIRETYPE_FIXED32,           // TYPE_FLOAT
		WIRETYPE_VARINT,            // TYPE_INT64
		WIRETYPE_VARINT,            // TYPE_UINT64
		WIRETYPE_VARINT,            // TYPE_INT32
		WIRETYPE_FIXED64,           // TYPE_FIXED64
		WIRETYPE_FIXED32,           // TYPE_FIXED32
		WIRETYPE_VARINT,            // TYPE_BOOL
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_STRING
		WIRETYPE_START_GROUP,       // TYPE_GROUP
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_MESSAGE
		WIRETYPE_LENGTH_DELIMITED,  // TYPE_BYTES
		WIRETYPE_VARINT,            // TYPE_UINT32
		WIRETYPE_VARINT,            // TYPE_ENUM
		WIRETYPE_FIXED32,           // TYPE_SFIXED32
		WIRETYPE_FIXED64,           // TYPE_SFIXED64
		WIRETYPE_VARINT,            // TYPE_SINT32
		WIRETYPE_VARINT,            // TYPE_SINT64
	};

	Tag tag;
	std::string value;
	std::string full;
public:
	Field& operator=(const Field& f) {
		this->tag = f.tag;
		this->value = f.value;
		this->full = f.full;
		return *this;
	}
	Field() { tag = { 0,0 }; value = ""; full = ""; }
	Field(unsigned field, unsigned type, std::string value, std::string full) {
		this->tag = { field, type };
		this->value = value;
		this->full = full;
	}
	Field(unsigned field, unsigned type, float value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = { field, (unsigned)wireType };
		full = getBytesVarint32(MAKE_TAG(field, wireType));
		full += std::string{ reinterpret_cast<const char*>(&value), 4 };
	}
	Field(unsigned field, unsigned type, double value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = { field, (unsigned)wireType };
		full = getBytesVarint32(MAKE_TAG(field, wireType));
		full += std::string{ reinterpret_cast<const char*>(&value), 8 };
	}
	Field(unsigned field, unsigned type, int64_t value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = { field, (unsigned)wireType };
		full = getBytesVarint32(MAKE_TAG(field, wireType));
		switch (wireType) {
		case WIRETYPE_VARINT:
			full += getBytesVarint64(value);
			break;
		case WIRETYPE_FIXED32:
			full += std::string{ reinterpret_cast<const char*>(&value), 4 };
			break;
		case WIRETYPE_FIXED64:
			full += std::string{ reinterpret_cast<const char*>(&value), 8 };
			break;
		}
	}
	Field(unsigned field, unsigned type, std::string value) {
		auto wireType = kWireTypeForFieldType[type];
		tag = { field, (unsigned)wireType };
		full = getBytesVarint32(MAKE_TAG(field, wireType));
		full += getBytesVarint32(value.size());
		full += value;
	}
public:
	float Float() {
		return *reinterpret_cast<float*>((void*)value.data());
	}
	double Double() {
		return *reinterpret_cast<double*>((void*)value.data());
	}
	int32_t Int32() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	int64_t Int64() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	uint32_t UInt32() {
		size_t bytesRead;
		return readVarUint32((void*)value.data(), bytesRead);
	}
	uint64_t UInt64() {
		size_t bytesRead;
		return readVarUint64((void*)value.data(), bytesRead);
	}
	uint32_t Fixed32() {
		return *reinterpret_cast<uint32_t*>((void*)value.data());
	}
	uint64_t Fixed64() {
		return *reinterpret_cast<uint64_t*>((void*)value.data());
	}
	int32_t SFixed32() {
		return *reinterpret_cast<int32_t*>((void*)value.data());
	}
	int64_t SFixed64() {
		return *reinterpret_cast<int64_t*>((void*)value.data());
	}
	bool Bool() {
		size_t bytesRead;
		return !!readVarUint32((void*)value.data(), bytesRead);
	}
	std::string String()
	{
		size_t bytesRead;
		void* data = (void*)value.data();
		auto length = readVarUint32((void*)value.data(), bytesRead);
		auto value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length };
		return value;
	}
public:
	static std::string getBytesVarint32(uint32_t value) {
		uint8_t bytes[kMaxVarint32Bytes];
		int size = 0;
		while (value > 0x7F) {
			bytes[size++] = (static_cast<uint8_t>(value) & 0x7F) | 0x80;
			value >>= 7;
		}
		bytes[size++] = static_cast<uint8_t>(value) & 0x7F;
		return std::string{ reinterpret_cast<const char*>(&bytes[0]), (size_t)size };
	}
	static std::string getBytesVarint64(uint64_t value) {
		uint8_t bytes[kMaxVarintBytes];
		int size = 0;
		while (value > 0x7F) {
			bytes[size++] = (static_cast<uint8_t>(value) & 0x7F) | 0x80;
			value >>= 7;
		}
		bytes[size++] = static_cast<uint8_t>(value) & 0x7F;
		return std::string{ reinterpret_cast<const char*>(&bytes[0]), (size_t)size };
	}
public:
	static const auto TagTypeBits = 3;
	static const auto TagTypeMask = (1 << 3) - 1;
	static const int kMaxVarintBytes = 10;
	static const int kMaxVarint32Bytes = 5;

	static Field read(void* data, size_t& bytesRead)
	{
		unsigned field = *reinterpret_cast<uint16_t*>(data);
		unsigned type = field & TagTypeMask;

		if (field == 0xffff) {
			bytesRead = 0;
			return Field();
		}

		if (field & 0x80) {
			field = ((field & 0x7f) | ((field & 0xff00) >> 1)) >> TagTypeBits;
			bytesRead = 2;
		}
		else {
			field = (field & 0xff) >> TagTypeBits;
			bytesRead = 1;
		}

		size_t length, sizeDelimited;
		std::string value, full;
		switch (type)
		{
		case WIRETYPE_VARINT:
			readVarUint64((void*)((ptrdiff_t)data + bytesRead), length);
			value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length };
			full = std::string{ reinterpret_cast<const char*>(data), bytesRead + length };
			bytesRead += length;
			break;
		case WIRETYPE_FIXED64:
			value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), 8 };
			full = std::string{ reinterpret_cast<const char*>(data), bytesRead + 8 };
			bytesRead += 8;
			break;
		case WIRETYPE_LENGTH_DELIMITED:
			sizeDelimited = readVarUint32((void*)((ptrdiff_t)data + bytesRead), length);
			value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), length + sizeDelimited };
			full = std::string{ reinterpret_cast<const char*>(data), bytesRead + length + sizeDelimited };
			bytesRead += length + sizeDelimited;
			break;
		case WIRETYPE_START_GROUP:
			throw("WIRETYPE_START_GROUP unrealised");
			break;
		case WIRETYPE_END_GROUP:
			throw("WIRETYPE_END_GROUP unrealised");
			break;
		case WIRETYPE_FIXED32:
			value = std::string{ reinterpret_cast<const char*>((void*)((ptrdiff_t)data + bytesRead)), 4 };
			full = std::string{ reinterpret_cast<const char*>(data), bytesRead + 4 };
			bytesRead += 4;
			break;
		default:
			throw("Unknown type %i", type);
			break;
		}


		return Field(field, type, value, full);
	}

	static uint32_t readVarUint32(void* data, size_t& bytesRead)
	{
		auto ptr = reinterpret_cast<const uint8_t*>(data);
		auto value = 0u;
		auto bytes = 0u;

		do {
			value |= static_cast<uint32_t>(*ptr & 0x7f) << (7 * bytes);
			bytes++;
		} while (*(ptr++) & 0x80 && bytes <= 5);

		bytesRead = bytes;
		return value;
	}

	static uint64_t readVarUint64(void* data, size_t& bytesRead)
	{
		auto ptr = reinterpret_cast<const uint8_t*>(data);
		auto value = 0ull;
		auto bytes = 0u;

		do
		{
			value |= static_cast<uint64_t>(*ptr & 0x7f) << (7 * bytes);
			bytes++;
		} while (*(ptr++) & 0x80 && bytes <= 10);

		bytesRead = bytes;

		return value;
	}
};

class Writer
{
    std::vector<std::vector<Field>> fields;
public:
    Writer() {}
    Writer(size_t maxFields) {
        size_t vector_size = maxFields + 1;
        fields.resize(vector_size);
        fields.reserve(vector_size);
    }

    Writer(void* data, size_t size, size_t maxFields)
        : Writer(maxFields)
    {
        const size_t vector_size = maxFields + 1;
        size_t
            pos = 0,
            bytesRead;

        if (data == nullptr)
            return;
        // parse packet
        while (pos < size) {
            auto field = Field::read((void*)((ptrdiff_t)data + pos), bytesRead);
            if (!bytesRead) break;

            auto index = field.tag.field;
            if (index >= vector_size) throw("fields range error: field[%i]", index);
            fields[index].push_back(field);
            pos += bytesRead;
        }
    }

    Writer(std::string dataStr, size_t maxFields)
        : Writer((void*)dataStr.data(), dataStr.size(), maxFields) {}

    std::string serialize() {
        std::string result;
        for (auto& f0 : fields) {
            for (auto& f1 : f0) {
                result += f1.full;
            }
        }
        return result;
    }

    void print() {
        auto data = serialize();
        void* mem = (void*)data.data();
        size_t size = data.size();
        int j = 0;
        for (int i = 0; i <= size; ++i) {
            printf("%.2X ", *(unsigned char*)((uintptr_t)mem + i));
            j++;
            if (j == 16)
            {
                j = 0;
                printf("\n");
            }
        }
        printf("\n");
    }

    void add(Field field) {
        fields[field.tag.field].push_back(field);
    }

    void replace(Field field) {
        fields[field.tag.field].clear();
        fields[field.tag.field].push_back(field);
    }

    void replace(Field field, uint32_t index) {
        fields[field.tag.field][index] = field;
    }

    void clear(unsigned fieldId) {
        return fields[fieldId].clear();
    }

    bool has(unsigned fieldId) {
        return fields[fieldId].size() > 0;
    }

    Field get(unsigned fieldId)
    {
        return fields[fieldId][0];
    }

    std::vector<Field> getAll(unsigned fieldId)
    {
        return fields[fieldId];
    }
};

EGCResults __fastcall Hooked_RetrieveMsg(void* _this, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	static auto Original = g_SteamAPI->GetOriginal<decltype(&Hooked_RetrieveMsg)>(2);
	return Original(_this, 0, punMsgType, pubDest, cubDest, pcubMsgSize);

#ifdef NU 

     auto WriteMsg = [](std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize) {
        if ((uint32_t)packet.size() <= cubDest - 8)
        {
            memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
            *pcubMsgSize = packet.size() + 8;
        }
        else
        {
            auto memPtr = *CASTMSG(void**, thisPtr, 0x14);
            auto memPtrSize = *CASTMSG(uint32_t*, thisPtr, 0x18);
            auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

            auto memory = interfaces.memalloc->Realloc(memPtr, newSize + 4);

            *CASTMSG(void**, thisPtr, 0x14) = memory;
            *CASTMSG(uint32_t*, thisPtr, 0x18) = newSize;
            *CASTMSG(void**, oldEBP, -0x14) = memory;

            memcpy(CASTMSG(void*, memory, 24), (void*)packet.data(), packet.size());

            *pcubMsgSize = packet.size() + 8;
        }
    };


     auto ChangeRank = [](void* pubDest, uint32_t* pcubMsgSize) {
        Writer msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 19);

        //replace commends
        auto _commendation = msg.has(_gc2ch::commendation) ? msg.get(_gc2ch::commendation).String() : std::string("");
        Writer commendation(_commendation, 4);

    
            commendation.replace(Field(_pci::cmd_friendly, TYPE_UINT32, (int64_t)128));


            commendation.replace(Field(_pci::cmd_teaching, TYPE_UINT32, (int64_t)256));


            commendation.replace(Field(_pci::cmd_leader, TYPE_UINT32, (int64_t)64));

        msg.replace(Field(_gc2ch::commendation, TYPE_STRING, commendation.serialize()));

     

       


        return msg.serialize();
    };


    auto status = Original(_this, 0, punMsgType, pubDest, cubDest, pcubMsgSize);

    if (status == k_EGCResultOK)
    {
        void* thisPtr = nullptr;
        __asm mov thisPtr, ebx;
        auto oldEBP = *reinterpret_cast<void**>((uint32_t)_AddressOfReturnAddress() - 4);

        uint32_t messageType = *punMsgType & 0x7FFFFFFF;

		if (messageType == k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello)
		{
			auto NewMsg = ChangeRank(pubDest, pcubMsgSize);
			WriteMsg(NewMsg, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
    }

    return status;
#endif
}
EGCResults __fastcall Hooked_SendSteamMsg(void* _this, void*, uint32_t unMsgType, const void* pubData, uint32_t cubData)
{
	static auto Original = g_SteamAPI->GetOriginal<decltype(&Hooked_SendSteamMsg)>(0);



	return Original(_this, 0, unMsgType, pubData, cubData);
}
void __fastcall Hooked_PacketStart(void* ecx, void*, int incoming, int outgoing) {
    static auto original_fn = g_pClientStateAdd->GetOriginal<PacketStartFn>(5);

    if (csgo->DoUnload)
        return original_fn(ecx, incoming, outgoing);

    if (!csgo->local->isAlive())
        return original_fn(ecx, incoming, outgoing);

    if (csgo->packets.empty())
        return original_fn(ecx, incoming, outgoing);

    for (auto it = csgo->packets.rbegin(); it != csgo->packets.rend(); ++it)
    {
        if (!it->is_outgoing)
            continue;

        if (it->cmd_number == outgoing || outgoing > it->cmd_number && (!it->is_used || it->previous_command_number == outgoing))
        {
            it->previous_command_number = outgoing;
            it->is_used = true;
            original_fn(ecx, incoming, outgoing);
            break;
        }
    }

    auto result = false;

    for (auto it = csgo->packets.begin(); it != csgo->packets.end();)
    {
        if (outgoing == it->cmd_number || outgoing == it->previous_command_number)
            result = true;

        if (outgoing > it->cmd_number && outgoing > it->previous_command_number)
            it = csgo->packets.erase(it);
        else
            ++it;
    }

    if (!result)
        original_fn(ecx, incoming, outgoing);
}

void __fastcall Hooked_PacketEnd(void* ecx, void* edx)
{
    static auto original_fn = g_pClientStateAdd->GetOriginal <PacketEndFn>(6);

    if (!csgo->local->isAlive())  //-V807
    {
        csgo->c_data.clear();
        return original_fn(ecx);
    }
    auto clientstate = (CClientState*)ecx;
    if (*(int*)((uintptr_t)ecx + 0x164) == *(int*)((uintptr_t)ecx + 0x16C))
    {
        auto ack_cmd = *(int*)((uintptr_t)ecx + 0x4D2C);
        auto correct = std::find_if(csgo->c_data.begin(), csgo->c_data.end(),
            [&ack_cmd](const correction_data& other_data)
            {
                return other_data.command_number == ack_cmd;
            }
        );

        auto netchannel = interfaces.engine->GetNetChannelInfo();

        if (netchannel && correct != csgo->c_data.end())
        {
            if (csgo->velocity_modifier > csgo->local->GetVelocityModifier() + 0.1f)
            {
                auto weapon = csgo->weapon;

                if (weapon || weapon->GetItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->IsNade()) //-V648
                {
                    for (auto& number : csgo->choked_number)
                    {
                        auto cmd = &interfaces.input->m_pCommands[number % 150];
                        auto verified = &interfaces.input->m_pVerifiedCommands[number % 150];

                        if (cmd->buttons & (IN_ATTACK | IN_ATTACK2))
                        {
                            cmd->buttons &= ~IN_ATTACK;

                            verified->m_cmd = *cmd;
                            verified->m_crc = cmd->GetChecksum();
                        }
                    }
                }
            }

            csgo->velocity_modifier = csgo->local->GetVelocityModifier();
        }
    }

    return original_fn(ecx);
}


int __fastcall Hooked_SendDatagram(void* netchan, void*, void *datagram)
{
	if (!H::SendDatagram)
		H::SendDatagram = csgo->g_pNetChannelHook->GetOriginal<send_datagram_fn>(46);

	if (!interfaces.engine->IsInGame() || csgo->DoUnload)
		return H::SendDatagram(netchan, datagram);

	const auto pNetChannel = reinterpret_cast<INetChannel*> (netchan);
	const auto backup_seqnr = pNetChannel->iInSequenceNr;
	g_Backtrack->DoPingSpike(pNetChannel);
	const auto ret = H::SendDatagram(pNetChannel, datagram);
	pNetChannel->iInSequenceNr = backup_seqnr;
	return ret;
}

bool __fastcall Hooked_SendNetMsg(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice)
{
    auto SendNetMsg = csgo->g_pNetChannelHook->GetOriginal<send_net_msg_fn>(40);

    if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
        return false;

    if (msg.GetGroup() == 9) // Fix lag when transmitting voice and fakelagging
        bVoice = true;

    return SendNetMsg(pNetChan, msg, bForceReliable, bVoice);
}

void __fastcall Hooked_ProcessPacket(void* ecx, void* edx, void* packet, bool header) {

    auto ProcessPacket = csgo->g_pNetChannelHook->GetOriginal<ProcessPacket_t>(39);
    if (!csgo->client_state->pNetChannel)
        return ProcessPacket(ecx, packet, header);

    ProcessPacket(ecx, packet, header);

    // get this from CL_FireEvents string "Failed to execute event for classId" in engine.dll
    for (CEventInfo* it{ csgo->client_state->pEvents }; it != nullptr; it = it->m_next) {
        if (!it->m_class_id)
            continue;

        // set all delays to instant.
        it->m_fire_delay = 0.f;
    }

    interfaces.engine->FireEvents();
}
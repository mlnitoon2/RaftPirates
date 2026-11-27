#include "rp/Quest.h"
#include <algorithm>

namespace rp {
    Quest::Quest() = default;
    Quest::~Quest() = default;

    void Quest::initWithSFSObject(const sfs::SFSObjectWrapper& state,
                                  const sfs::SFSObjectWrapper& definition) {
        id = state.getInt("id", state.getInt("quest_id"));
        unread = state.getBool("new", state.getInt("new") != 0);
        name = definition.getString("name");
        description = definition.getString("description");
        type = definition.getInt("quest_type_id");
        visible = definition.getBool("visible", definition.getInt("visible", 1) != 0);
        sheet = definition.getString("sheet_id");
        image = definition.getString("image_id");
        highlightFeature = definition.getString("highlight_feature");
        popupImmediately = definition.getBool("popup_immediately", definition.getInt("popup_immediately") != 0);
        group = definition.getInt("group");
        sheetLeft = definition.getString("sheet_id_left");
        imageLeft = definition.getString("image_id_left");
        sheetBottom = definition.getString("sheet_id_bottom");
        imageBottom = definition.getString("image_id_bottom");
        setProgressPercentage(state.getInt("progress", state.getInt("percentage")));
        opened = false;
    }

    void Quest::markRead() {
        unread = false;
    }

    void Quest::markOpened() {
        opened = true;
        markRead();
    }

    void Quest::setProgressPercentage(int value) {
        progress = std::clamp(value, 0, 100);
    }
} // namespace rp

namespace rp {
    QuestTypeInfo::QuestTypeInfo(const sfs::SFSObjectWrapper& row)
        : id(row.getInt("type_id")), name(row.getString("name")),
          sheet(row.getString("sheet_id")), image(row.getString("image_id")) {}
    QuestTypeInfo::~QuestTypeInfo() = default;
} // namespace rp

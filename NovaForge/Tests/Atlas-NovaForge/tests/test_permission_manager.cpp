#include "../engine/core/PermissionManager.h"
#include <iostream>
#include <cassert>

using namespace atlas;

void test_permission_defaults() {
    PermissionManager pm;
    assert(pm.GetDefaultTier() == PermissionTier::Developer);
    assert(pm.UserCount() == 0);

    pm.SetDefaultTier(PermissionTier::QA);
    assert(pm.GetDefaultTier() == PermissionTier::QA);

    std::cout << "[PASS] test_permission_defaults" << std::endl;
}

void test_permission_register_user() {
    PermissionManager pm;
    pm.RegisterUser("alice", PermissionTier::Developer);
    pm.RegisterUser("bob", PermissionTier::QA);

    assert(pm.UserCount() == 2);
    assert(pm.GetUserTier("alice") == PermissionTier::Developer);
    assert(pm.GetUserTier("bob") == PermissionTier::QA);

    std::cout << "[PASS] test_permission_register_user" << std::endl;
}

void test_permission_tier_viewonly() {
    PermissionManager pm;
    pm.RegisterUser("viewer", PermissionTier::ViewOnly);

    assert(pm.HasPermission("viewer", Permission::ViewState));
    assert(pm.HasPermission("viewer", Permission::InspectEntities));
    assert(pm.HasPermission("viewer", Permission::ViewReplays));

    assert(!pm.HasPermission("viewer", Permission::RunTests));
    assert(!pm.HasPermission("viewer", Permission::EditAssets));
    assert(!pm.HasPermission("viewer", Permission::FullAccess));

    auto perms = PermissionManager::TierPermissions(PermissionTier::ViewOnly);
    assert(perms.size() == 3);

    std::cout << "[PASS] test_permission_tier_viewonly" << std::endl;
}

void test_permission_tier_developer() {
    PermissionManager pm;
    pm.RegisterUser("dev", PermissionTier::Developer);

    // Developer has all QA perms plus dev-specific ones
    assert(pm.HasPermission("dev", Permission::ViewState));
    assert(pm.HasPermission("dev", Permission::RunTests));
    assert(pm.HasPermission("dev", Permission::EditAssets));
    assert(pm.HasPermission("dev", Permission::EditGraphs));
    assert(pm.HasPermission("dev", Permission::ModifyState));
    assert(pm.HasPermission("dev", Permission::UseDebugTools));

    assert(!pm.HasPermission("dev", Permission::RunCI));
    assert(!pm.HasPermission("dev", Permission::FullAccess));

    auto perms = PermissionManager::TierPermissions(PermissionTier::Developer);
    assert(perms.size() == 10);

    std::cout << "[PASS] test_permission_tier_developer" << std::endl;
}

void test_permission_tier_admin() {
    PermissionManager pm;
    pm.RegisterUser("root", PermissionTier::Admin);

    assert(pm.HasPermission("root", Permission::ViewState));
    assert(pm.HasPermission("root", Permission::RunTests));
    assert(pm.HasPermission("root", Permission::EditAssets));
    assert(pm.HasPermission("root", Permission::RunCI));
    assert(pm.HasPermission("root", Permission::FullAccess));

    assert(PermissionManager::TierName(PermissionTier::Admin) == "Admin");
    assert(PermissionManager::TierName(PermissionTier::ViewOnly) == "ViewOnly");
    assert(PermissionManager::TierName(PermissionTier::CI) == "CI");

    std::cout << "[PASS] test_permission_tier_admin" << std::endl;
}

void test_permission_grant_custom() {
    PermissionManager pm;
    pm.RegisterUser("viewer", PermissionTier::ViewOnly);

    assert(!pm.HasPermission("viewer", Permission::RunTests));

    pm.GrantPermission("viewer", Permission::RunTests);
    assert(pm.HasPermission("viewer", Permission::RunTests));

    // Granting the same permission again should not duplicate
    pm.GrantPermission("viewer", Permission::RunTests);
    assert(pm.HasPermission("viewer", Permission::RunTests));

    std::cout << "[PASS] test_permission_grant_custom" << std::endl;
}

void test_permission_revoke() {
    PermissionManager pm;
    pm.RegisterUser("viewOnlyUser", PermissionTier::ViewOnly);

    pm.GrantPermission("viewOnlyUser", Permission::RunTests);
    pm.GrantPermission("viewOnlyUser", Permission::EditAssets);
    assert(pm.HasPermission("viewOnlyUser", Permission::RunTests));
    assert(pm.HasPermission("viewOnlyUser", Permission::EditAssets));

    pm.RevokePermission("viewOnlyUser", Permission::RunTests);
    assert(!pm.HasPermission("viewOnlyUser", Permission::RunTests));
    assert(pm.HasPermission("viewOnlyUser", Permission::EditAssets));

    std::cout << "[PASS] test_permission_revoke" << std::endl;
}

void test_permission_user_management() {
    PermissionManager pm;
    pm.RegisterUser("alice", PermissionTier::Developer);
    pm.RegisterUser("bob", PermissionTier::QA);
    pm.RegisterUser("charlie", PermissionTier::Admin);

    assert(pm.UserCount() == 3);

    auto users = pm.RegisteredUsers();
    assert(users.size() == 3);

    pm.RemoveUser("bob");
    assert(pm.UserCount() == 2);
    assert(!pm.HasPermission("bob", Permission::ViewState));

    // Unregistered user gets default tier from GetUserTier
    assert(pm.GetUserTier("unknown") == pm.GetDefaultTier());

    std::cout << "[PASS] test_permission_user_management" << std::endl;
}


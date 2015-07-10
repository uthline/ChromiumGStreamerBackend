// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.customtabs;

import android.app.Activity;
import android.app.PendingIntent;
import android.app.PendingIntent.CanceledException;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.IBinder;
import android.support.customtabs.CustomTabsIntent;
import android.text.TextUtils;
import android.util.Pair;

import org.chromium.base.Log;
import org.chromium.base.VisibleForTesting;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.util.IntentUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * A model class that parses intent from third-party apps and provides results to
 * {@link CustomTabActivity}.
 */
public class CustomTabIntentDataProvider {
    private static final String TAG = "CustomTabIntentData";

    /**
     * Extra used to keep the caller alive. Its value is an Intent.
     */
    public static final String EXTRA_KEEP_ALIVE = "android.support.customtabs.extra.KEEP_ALIVE";

    /**
     * Extra int that specifies the style of the back button on the toolbar. Default is showing a
     * cross icon.
     */
    public static final String EXTRA_CLOSE_BUTTON_STYLE =
            "android.support.customtabs.extra.CLOSE_BUTTON_STYLE";

    /**
     * Uses 'X'-like cross icon for close button.
     */
    public static final int CLOSE_BUTTON_CROSS = 0;

    /**
     * Uses '<'-like chevron arrow icon for close button.
     */
    public static final int CLOSE_BUTTON_ARROW = 1;

    /**
     * Extra int that specifies state for showing the page title. Default is showing only the domain
     * and no information about the title.
     */
    public static final String EXTRA_TITLE_VISIBILITY_STATE =
            "android.support.customtabs.extra.TITLE_VISIBILITY";

    /**
     * Don't show any title. Shows only the domain.
     */
    public static final int NO_TITLE = 0;

    /**
     * Shows the page title and the domain.
     */
    public static final int SHOW_PAGE_TITLE = 1;

    private static final String BUNDLE_PACKAGE_NAME = "android:packageName";
    private static final String BUNDLE_ENTER_ANIMATION_RESOURCE = "android:animEnterRes";
    private static final String BUNDLE_EXIT_ANIMATION_RESOURCE = "android:animExitRes";
    private final IBinder mSession;
    private final Intent mKeepAliveServiceIntent;
    private final int mTitleVisibilityState;
    private final int mCloseButtonResId;
    private int mToolbarColor;
    private Bitmap mIcon;
    private PendingIntent mActionButtonPendingIntent;
    private List<Pair<String, PendingIntent>> mMenuEntries = new ArrayList<>();
    private Bundle mAnimationBundle;
    // OnFinished listener for PendingIntents. Used for testing only.
    private PendingIntent.OnFinished mOnFinished;

    /**
     * Constructs a {@link CustomTabIntentDataProvider}.
     */
    public CustomTabIntentDataProvider(Intent intent, Context context) {
        if (intent == null) assert false;

        mSession = IntentUtils.safeGetBinderExtra(intent, CustomTabsIntent.EXTRA_SESSION);
        retrieveToolbarColor(intent, context);
        mKeepAliveServiceIntent = IntentUtils.safeGetParcelableExtra(intent, EXTRA_KEEP_ALIVE);

        Bundle actionButtonBundle =
                IntentUtils.safeGetBundleExtra(intent, CustomTabsIntent.EXTRA_ACTION_BUTTON_BUNDLE);
        if (actionButtonBundle != null) {
            mIcon = IntentUtils.safeGetParcelable(actionButtonBundle, CustomTabsIntent.KEY_ICON);
            if (mIcon != null && !checkBitmapSizeWithinBounds(context, mIcon)) {
                mIcon.recycle();
                mIcon = null;
            } else if (mIcon != null) {
                mActionButtonPendingIntent = IntentUtils.safeGetParcelable(
                        actionButtonBundle, CustomTabsIntent.KEY_PENDING_INTENT);
            }
        }

        List<Bundle> menuItems =
                IntentUtils.getParcelableArrayListExtra(intent, CustomTabsIntent.EXTRA_MENU_ITEMS);
        if (menuItems != null) {
            for (Bundle bundle : menuItems) {
                String title =
                        IntentUtils.safeGetString(bundle, CustomTabsIntent.KEY_MENU_ITEM_TITLE);
                PendingIntent pendingIntent =
                        IntentUtils.safeGetParcelable(bundle, CustomTabsIntent.KEY_PENDING_INTENT);
                if (TextUtils.isEmpty(title) || pendingIntent == null) continue;
                mMenuEntries.add(new Pair<String, PendingIntent>(title, pendingIntent));
            }
        }

        mAnimationBundle = IntentUtils.safeGetBundleExtra(
                intent, CustomTabsIntent.EXTRA_EXIT_ANIMATION_BUNDLE);

        int closeButtonStyle =
                IntentUtils.safeGetIntExtra(intent, EXTRA_CLOSE_BUTTON_STYLE, CLOSE_BUTTON_CROSS);
        switch(closeButtonStyle) {
            case CustomTabIntentDataProvider.CLOSE_BUTTON_ARROW:
                mCloseButtonResId = R.drawable.btn_chevron_left;
                break;
            case CustomTabIntentDataProvider.CLOSE_BUTTON_CROSS:
            default:
                mCloseButtonResId = R.drawable.btn_close;
        }

        mTitleVisibilityState =
                IntentUtils.safeGetIntExtra(intent, EXTRA_TITLE_VISIBILITY_STATE, NO_TITLE);
    }

    /**
     * Processes the color passed from the client app and updates {@link #mToolbarColor}.
     */
    private void retrieveToolbarColor(Intent intent, Context context) {
        int color = IntentUtils.safeGetIntExtra(intent, CustomTabsIntent.EXTRA_TOOLBAR_COLOR,
                context.getResources().getColor(R.color.default_primary_color));
        int defaultColor = context.getResources().getColor(R.color.default_primary_color);

        if (color == Color.TRANSPARENT) color = defaultColor;

        // Ignore any transparency value.
        color |= 0xFF000000;

        mToolbarColor = color;
    }

    /**
     * @return The session specified in the intent, or null.
     */
    public IBinder getSession() {
        return mSession;
    }

    /**
     * @return The keep alive service intent specified in the intent, or null.
     */
    public Intent getKeepAliveServiceIntent() {
        return mKeepAliveServiceIntent;
    }

    /**
     * @return The toolbar color specified in the intent. Will return the color of
     *         default_primary_color, if not set in the intent.
     */
    public int getToolbarColor() {
        return mToolbarColor;
    }

    /**
     * @return The resource id of the close button shown in the custom tab toolbar.
     */
    public int getCloseButtonIconResId() {
        return mCloseButtonResId;
    }

    /**
     * @return The title visibility state for the toolbar.
     *         Default is {@link CustomTabIntentDataProvider#CUSTOM_TAB_NO_TITLE}.
     */
    public int getTitleVisibilityState() {
        return mTitleVisibilityState;
    }

    /**
     * @return Whether the client app has provided sufficient info for the toolbar to show the
     *         action button.
     */
    public boolean shouldShowActionButton() {
        return mIcon != null && mActionButtonPendingIntent != null;
    }

    /**
     * @return The icon used for the action button. Will be null if not set in the intent.
     */
    public Bitmap getActionButtonIcon() {
        return mIcon;
    }

    /**
     * @return The {@link PendingIntent} that will be sent when the user clicks the action button.
     *         For testing only.
     */
    @VisibleForTesting
    public PendingIntent getActionButtonPendingIntentForTest() {
        return mActionButtonPendingIntent;
    }

    /**
     * @return Titles of menu items that were passed from client app via intent.
     */
    public List<String> getMenuTitles() {
        ArrayList<String> list = new ArrayList<>();
        for (Pair<String, PendingIntent> pair : mMenuEntries) {
            list.add(pair.first);
        }
        return list;
    }

    /**
     * Triggers the client-defined action when the user clicks a custom menu item.
     * @param menuIndex The index that the menu item is shown in the result of
     *                  {@link #getMenuTitles()}
     */
    public void clickMenuItemWithUrl(Context context, int menuIndex, String url) {
        Intent addedIntent = new Intent();
        addedIntent.setData(Uri.parse(url));
        try {
            PendingIntent pendingIntent = mMenuEntries.get(menuIndex).second;
            pendingIntent.send(context, 0, addedIntent, mOnFinished, null);
        } catch (CanceledException e) {
            Log.e(TAG, "Custom tab in Chrome failed to send pending intent.");
        }
    }

    /**
     * @return Whether chrome should animate when it finishes. We show animations only if the client
     *         app has supplied the correct animation resources via intent extra.
     */
    public boolean shouldAnimateOnFinish() {
        return mAnimationBundle != null && getClientPackageName() != null;
    }

    /**
     * @return The package name of the client app. This is used for a workaround in order to
     *         retrieve the client's animation resources.
     */
    public String getClientPackageName() {
        if (mAnimationBundle == null) return null;
        return mAnimationBundle.getString(BUNDLE_PACKAGE_NAME);
    }

    /**
     * @return The resource id for enter animation, which is used in
     *         {@link Activity#overridePendingTransition(int, int)}.
     */
    public int getAnimationEnterRes() {
        return shouldAnimateOnFinish() ? mAnimationBundle.getInt(BUNDLE_ENTER_ANIMATION_RESOURCE)
                : 0;
    }

    /**
     * @return The resource id for exit animation, which is used in
     *         {@link Activity#overridePendingTransition(int, int)}.
     */
    public int getAnimationExitRes() {
        return shouldAnimateOnFinish() ? mAnimationBundle.getInt(BUNDLE_EXIT_ANIMATION_RESOURCE)
                : 0;
    }

    /**
     * Send the pending intent for the current action button with the given url as data.
     * @param context The context to use for sending the {@link PendingIntent}.
     * @param url The url to attach as additional data to the {@link PendingIntent}.
     */
    public void sendButtonPendingIntentWithUrl(Context context, String url) {
        assert mActionButtonPendingIntent != null;
        Intent addedIntent = new Intent();
        addedIntent.setData(Uri.parse(url));
        try {
            mActionButtonPendingIntent.send(context, 0, addedIntent, mOnFinished, null);
        } catch (CanceledException e) {
            Log.e(TAG, "CanceledException while sending pending intent in custom tab");
        }
    }

    private boolean checkBitmapSizeWithinBounds(Context context, Bitmap bitmap) {
        int height = context.getResources().getDimensionPixelSize(R.dimen.toolbar_icon_height);
        if (bitmap.getHeight() < height) return false;
        int scaledWidth = bitmap.getWidth() / bitmap.getHeight() * height;
        if (scaledWidth > 2 * height) return false;
        return true;
    }

    /**
     * Set the callback object for {@link PendingIntent}s that are sent in this class. For testing
     * purpose only.
     */
    @VisibleForTesting
    void setPendingIntentOnFinishedForTesting(PendingIntent.OnFinished onFinished) {
        mOnFinished = onFinished;
    }
}

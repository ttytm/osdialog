#include "osdialog.h"
#include <assert.h>
#include <string.h>
#include <gtk/gtk.h>



int osdialog_message(osdialog_message_level level, osdialog_message_buttons buttons, const char *message) {
	assert(gtk_init_check(NULL, NULL));

	GtkMessageType messageType;
	switch (level) {
		default:
		case OSDIALOG_INFO: messageType = GTK_MESSAGE_INFO; break;
		case OSDIALOG_WARNING: messageType = GTK_MESSAGE_WARNING; break;
		case OSDIALOG_ERROR: messageType = GTK_MESSAGE_ERROR; break;
	}

	GtkButtonsType buttonsType;
	switch (buttons) {
		default:
		case OSDIALOG_OK: buttonsType = GTK_BUTTONS_OK; break;
		case OSDIALOG_OK_CANCEL: buttonsType = GTK_BUTTONS_OK_CANCEL; break;
		case OSDIALOG_YES_NO: buttonsType = GTK_BUTTONS_YES_NO; break;
	}

	GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, messageType, buttonsType, "%s", message);

	gint result = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();

	return (result == GTK_RESPONSE_OK || result == GTK_RESPONSE_YES);
}


char *osdialog_prompt(osdialog_message_level level, const char *message, const char *text) {
	// TODO
	(void) level;
	(void) message;
	(void) text;
	assert(0);
	return NULL;
}


char *osdialog_file(osdialog_file_action action, const char *path, const char *filename, osdialog_filters *filters) {
	assert(gtk_init_check(NULL, NULL));

	GtkFileChooserAction gtkAction;
	const char *title;
	const char *acceptText;
	if (action == OSDIALOG_OPEN) {
		title = "Open File";
		acceptText = "Open";
		gtkAction = GTK_FILE_CHOOSER_ACTION_OPEN;
	}
	else if (action == OSDIALOG_OPEN_DIR) {
		title = "Open Folder";
		acceptText = "Open Folder";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	}
	else {
		title = "Save File";
		acceptText = "Save";
		gtkAction = GTK_FILE_CHOOSER_ACTION_SAVE;
	}

	GtkWidget *dialog = gtk_file_chooser_dialog_new(
		title,
		NULL,
		gtkAction,
		"Cancel",
		GTK_RESPONSE_CANCEL,
		acceptText,
		GTK_RESPONSE_ACCEPT,
		NULL);

	for (; filters; filters = filters->next) {
		GtkFileFilter *fileFilter = gtk_file_filter_new();
		gtk_file_filter_set_name(fileFilter, filters->name);
		for (osdialog_filter_patterns *patterns = filters->patterns; patterns; patterns = patterns->next) {
			char patternBuf[1024];
			snprintf(patternBuf, sizeof(patternBuf), "*.%s", patterns->pattern);
			gtk_file_filter_add_pattern(fileFilter, patternBuf);
		}
		gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), fileFilter);
	}

	if (action == OSDIALOG_SAVE)
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (path)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);

	if (action == OSDIALOG_SAVE && filename)
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);

	char *chosen_filename = NULL;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		chosen_filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}
	gtk_widget_destroy(dialog);

	char *result = NULL;
	if (chosen_filename) {
		result = osdialog_strndup(chosen_filename, strlen(chosen_filename));
		g_free(chosen_filename);
	}

	while (gtk_events_pending())
		gtk_main_iteration();
	return result;
}


int osdialog_color_picker(osdialog_color *color, int opacity) {
	if (!color)
		return 0;
	assert(gtk_init_check(NULL, NULL));

	GtkWidget *dialog = gtk_color_chooser_dialog_new ("Color",NULL);
	GtkColorChooser *color_chooser = GTK_COLOR_CHOOSER(dialog);

	GdkRGBA rgba;
	rgba.red = color->r / 255.0;
	rgba.green = color->g / 255.0;
	rgba.blue = color->b / 255.0;
	rgba.alpha = color->a / 255.0;
   
	gtk_color_chooser_set_rgba (color_chooser, &rgba);
	gtk_color_chooser_set_use_alpha(color_chooser, opacity);

	int result = 0;
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		GdkRGBA c;
		gtk_color_chooser_get_rgba(color_chooser, &c);
		color->r = c.red * 65535 + 0.5;
		color->g = c.green * 65535 + 0.5;
		color->b = c.blue * 65535 + 0.5;
		color->a = c.alpha * 65535 + 0.5;
		result = 1;
	}
	gtk_widget_destroy(dialog);

	while (gtk_events_pending())
		gtk_main_iteration();
	return result;
}

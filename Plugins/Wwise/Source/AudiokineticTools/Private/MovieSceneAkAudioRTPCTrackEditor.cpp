// Copyright (c) 2006-2016 Audiokinetic Inc. / All Rights Reserved

#include "MovieSceneAkAudioRTPCTrackEditor.h"
#include "AkAudioDevice.h"
#include "InterpTrackAkAudioRTPC.h"

#include "MovieScene.h"
#include "MovieSceneCommonHelpers.h"
#include "MovieSceneAkAudioRTPCTrack.h"
#include "MovieSceneAkAudioRTPCSection.h"

#include "SequencerUtilities.h"
#include "ISequencerSection.h"
#include "ISequencerObjectChangeListener.h"
#include "ISectionLayoutBuilder.h"
#include "SequencerSectionPainter.h"

#include "AkMatineeImportTools.h"
#include "AkAudioStyle.h"

#include "ScopedTransaction.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"

#include "EditorStyleSet.h"
#include "Editor.h"

#if !UE_4_20_OR_LATER
#include "FloatCurveKeyArea.h"
#endif

#define LOCTEXT_NAMESPACE "MovieSceneAkAudioRTPCTrackEditor"


/**
 * Class that draws a transform section in the sequencer
 */
class FMovieSceneAkAudioRTPCSection
	: public ISequencerSection
{
public:

	FMovieSceneAkAudioRTPCSection(UMovieSceneSection& InSection)
		: Section(Cast<UMovieSceneAkAudioRTPCSection>(&InSection))
	{ }

public:

	// ISequencerSection interface

	virtual UMovieSceneSection* GetSectionObject() override { return Section; }

#if !UE_4_17_OR_LATER
	virtual FText GetDisplayName() const override
	{
		return LOCTEXT("DisplayName", "AkAudioRTPC");
	}
#endif // !UE_4_17_OR_LATER

	virtual FText GetSectionTitle() const override
	{
		return FText::FromString(Section->GetRTPCName());
	}

#if !UE_4_20_OR_LATER
	virtual void GenerateSectionLayout(class ISectionLayoutBuilder& LayoutBuilder) const override
	{
		static const FLinearColor KeyAreaColor(0.0f, 0.4282f, 0.7f, 0.5f);
		KeyArea = MakeShareable(new FFloatCurveKeyArea(&Section->GetFloatCurve(), Section, KeyAreaColor));
		LayoutBuilder.SetSectionAsKeyArea(KeyArea.ToSharedRef());
	}
#endif

	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override
	{
		return InPainter.PaintSectionBackground();
	}

private:

	/** The section we are visualizing */
	UMovieSceneAkAudioRTPCSection* Section;

#if !UE_4_20_OR_LATER
	mutable TSharedPtr<FFloatCurveKeyArea> KeyArea;
#endif

};

FMovieSceneAkAudioRTPCTrackEditor::FMovieSceneAkAudioRTPCTrackEditor(TSharedRef<ISequencer> InSequencer)
#if UE_4_20_OR_LATER
	: FKeyframeTrackEditor<UMovieSceneAkAudioRTPCTrack>(InSequencer)
#else
	: FKeyframeTrackEditor<UMovieSceneAkAudioRTPCTrack, UMovieSceneAkAudioRTPCSection, float>(InSequencer)
#endif
{
}

TSharedRef<ISequencerTrackEditor> FMovieSceneAkAudioRTPCTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FMovieSceneAkAudioRTPCTrackEditor(InSequencer));
}

static void CopyInterpAkAudioRTPCTrack(TSharedRef<ISequencer> Sequencer, const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack, UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack)
{
	switch (FAkMatineeImportTools::CopyInterpAkAudioRTPCTrack(MatineeAkAudioRTPCTrack, AkAudioRTPCTrack))
	{
	case ECopyInterpAkAudioResult::NoChange:
		break;

	case ECopyInterpAkAudioResult::KeyModification:
		Sequencer.Get().NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::TrackValueChanged);
		break;

	case ECopyInterpAkAudioResult::SectionAdded:
		Sequencer.Get().NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
		break;
	}
}

void FMovieSceneAkAudioRTPCTrackEditor::BuildTrackContextMenu(FMenuBuilder& MenuBuilder, UMovieSceneTrack* Track)
{
	const UInterpTrackAkAudioRTPC* MatineeAkAudioRTPCTrack = FAkMatineeImportTools::GetTrackFromMatineeCopyPasteBuffer<UInterpTrackAkAudioRTPC>();
	UMovieSceneAkAudioRTPCTrack* AkAudioRTPCTrack = Cast<UMovieSceneAkAudioRTPCTrack>(Track);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("PasteMatineeMatineeAkAudioRTPCTrack", "Paste Matinee AkAudioRTPC Track"),
		LOCTEXT("PasteMatineeMatineeAkAudioRTPCTrackTooltip", "Pastes keys from a Matinee AkAudioRTPC track into this track."),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&CopyInterpAkAudioRTPCTrack, GetSequencer().ToSharedRef(), MatineeAkAudioRTPCTrack, AkAudioRTPCTrack),
			FCanExecuteAction::CreateLambda([=]{ return MatineeAkAudioRTPCTrack && AkAudioRTPCTrack && MatineeAkAudioRTPCTrack->GetNumKeyframes() > 0; })
		)
	);
}

TSharedRef<ISequencerSection> FMovieSceneAkAudioRTPCTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShareable(new FMovieSceneAkAudioRTPCSection(SectionObject));
}

const FSlateBrush* FMovieSceneAkAudioRTPCTrackEditor::GetIconBrush() const
{
	return FAkAudioStyle::Get().GetBrush("AudiokineticTools.RTPCIcon");
}



struct FRTPCSectionCreateDialogOptions
{
	FString RTPCName;
	bool OkClicked;

	FRTPCSectionCreateDialogOptions() : OkClicked(false) {}

	bool Validate()
	{
		// TODO: Verify that this RTPCName is a valid RTPC name!
		return OkClicked && RTPCName.Len() > 0;
	}
};


class SCreateAkAudioRTPCSectionDialog
	: public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SCreateAkAudioRTPCSectionDialog) { }
	SLATE_END_ARGS()

	/** Construct this widget. */
	void Construct(const FArguments& InArgs, FRTPCSectionCreateDialogOptions& InOptions, TSharedRef<SWindow> InWindow)
	{
		Options = &InOptions;
		Window = InWindow;

		ChildSlot
		[
			SNew(SBorder)
			.Visibility(EVisibility::Visible)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1)
				.VAlign(VAlign_Top)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					.Content()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2, 2, 6, 2)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("AkAudioRTPCName", "RTPC Name"))
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2, 0, 2, 0)
						[
							SNew(SEditableTextBox)
							.HintText(LOCTEXT("AkAudioRTPCNameHint", "Name of the AkAudioRTPC"))
							.OnTextCommitted(this, &SCreateAkAudioRTPCSectionDialog::OnEventNameCommited)
							.OnTextChanged(this, &SCreateAkAudioRTPCSectionDialog::OnEventNameCommited, ETextCommit::Default)
							.MinDesiredWidth(200)
							.RevertTextOnEscape(true)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Bottom)
				.Padding(8)
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
					.MinDesiredSlotWidth(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotWidth"))
					.MinDesiredSlotHeight(FEditorStyle::GetFloat("StandardDialog.MinDesiredSlotHeight"))

					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked_Lambda([this]() -> FReply { CloseDialog(true); return FReply::Handled(); })
						.Text(LOCTEXT("OkButtonLabel", "OK"))
					]

					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.OnClicked_Lambda([this]() -> FReply { CloseDialog(false); return FReply::Handled(); })
						.Text(LOCTEXT("CancelButtonLabel", "Cancel"))
					]
				]
			]
		];
	}

protected:

	void CloseDialog(bool InOkClicked)
	{
		Options->OkClicked = InOkClicked;

		if (Window.IsValid())
		{
			Window.Pin()->RequestDestroyWindow();
		}
	}

private:

	void OnEventNameCommited(const FText& InText, ETextCommit::Type InCommitType)
	{
		Options->RTPCName = InText.ToString();

		if (InCommitType == ETextCommit::OnEnter || InCommitType == ETextCommit::OnCleared)
		{
			CloseDialog(InCommitType == ETextCommit::OnEnter);
		}
	}


	FRTPCSectionCreateDialogOptions* Options;
	TWeakPtr<SWindow> Window;
};


bool ConfigureRTPCSection(FRTPCSectionCreateDialogOptions& Options)
{
	TSharedRef<SWindow> Window = SNew(SWindow)
	.Title(LOCTEXT("CreateAkAudioRTPCSectionDialog", "Enter AkAudioRTPC Name"))
	.ClientSize(FVector2D(372, 108))
	.SupportsMinimize(false)
	.SupportsMaximize(false);

	Window->SetContent(SNew(SCreateAkAudioRTPCSectionDialog, Options, Window));
	GEditor->EditorAddModalWindow(Window);

	return Options.Validate();
}



void FMovieSceneAkAudioRTPCTrackEditor::TryAddAkAudioRTPCTrack(FCreateAkAudioRTPCTrack DoCreateAkAudioRTPCTrack)
{
	FRTPCSectionCreateDialogOptions Options;
	if (ConfigureRTPCSection(Options))
	{
		auto FocusedMovieScene = GetFocusedMovieScene();

		if (FocusedMovieScene == nullptr)
		{
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("AddAkAudioRTPCTrack_Transaction", "Add AkAudioRTPC Track"));
		FocusedMovieScene->Modify();

		auto NewTrack = DoCreateAkAudioRTPCTrack.Execute(FocusedMovieScene);
		ensure(NewTrack);

		auto NewSection = NewTrack->CreateNewSection();
		ensure(NewSection);

		auto RTPCSection = Cast<UMovieSceneAkAudioRTPCSection>(NewSection);
		ensure(RTPCSection);
		RTPCSection->SetRTPCName(Options.RTPCName);

#if UE_4_20_OR_LATER
		NewSection->SetRange(TRange<FFrameNumber>::All());
#else
		NewSection->SetIsInfinite(true);
#endif
		NewTrack->AddSection(*NewSection);

		GetSequencer()->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
}

void FMovieSceneAkAudioRTPCTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	auto CreateAkAudioRTPCTrack = [=](UMovieScene* MovieScene)
	{
		auto NewTrack = MovieScene->AddMasterTrack<UMovieSceneAkAudioRTPCTrack>();
		if (NewTrack != nullptr)
		{
			NewTrack->SetIsAMasterTrack(true);
		}
		return NewTrack;
	};

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddAkAudioRTPCTrack", "AkAudioRTPC"),
		LOCTEXT("AddAkAudioRTPCMasterTrackTooltip", "Adds a master AkAudioRTPC track."),
		FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.RTPCIcon"),
		FUIAction(FExecuteAction::CreateLambda([=]
		{
			TryAddAkAudioRTPCTrack(FCreateAkAudioRTPCTrack::CreateLambda(CreateAkAudioRTPCTrack));
		}))
	);
}

bool FMovieSceneAkAudioRTPCTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	static UClass* LevelSequenceClass = FindObject<UClass>(ANY_PACKAGE, TEXT("LevelSequence"), true);
	return InSequence != nullptr && LevelSequenceClass != nullptr && InSequence->GetClass()->IsChildOf(LevelSequenceClass);
}


#if UE_4_23_OR_LATER
void FMovieSceneAkAudioRTPCTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const TArray<FGuid>& ObjectBindings, const UClass* ObjectClass)
{
	auto ObjectBinding = ObjectBindings[0];
#else
void FMovieSceneAkAudioRTPCTrackEditor::BuildObjectBindingTrackMenu(FMenuBuilder& MenuBuilder, const FGuid& ObjectBinding, const UClass* ObjectClass)
{
#endif
	if (!ObjectClass->IsChildOf(AActor::StaticClass()) && !ObjectClass->IsChildOf(USceneComponent::StaticClass()))
	{
		return;
	}

	auto CreateAkAudioRTPCTrack = [=](UMovieScene* MovieScene) { return MovieScene->AddTrack<UMovieSceneAkAudioRTPCTrack>(ObjectBinding); };

	MenuBuilder.AddMenuEntry(
		LOCTEXT("AddAkAudioRTPCTrack", "AkAudioRTPC"),
		LOCTEXT("AddAkAudioRTPCTrackTooltip", "Adds an AkAudioRTPC track."),
		FSlateIcon(FAkAudioStyle::GetStyleSetName(), "AudiokineticTools.RTPCIcon"),
		FUIAction(FExecuteAction::CreateLambda([=]
		{
			TryAddAkAudioRTPCTrack(FCreateAkAudioRTPCTrack::CreateLambda(CreateAkAudioRTPCTrack));
		}))
	);
}

#undef LOCTEXT_NAMESPACE
